//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageClassifier.h"

//CPP_2_________________________________ This project's headers
#include "FOVImageGrabber.h"
#include "VisionMainTR.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ImageClassifier_Info::ImageClassifier_Info(LPCTSTR trayKey, const std::vector<long>& needStitchList, long runtimeIndex,
    long trayIndex, long inspectionID, long paneCount)
    : m_trayKey(trayKey)
    , m_runtimeIndex(runtimeIndex)
    , m_trayIndex(trayIndex)
    , m_inspectionID(inspectionID)
    , m_paneCount(paneCount)
{
    for (auto& stitchIndex : needStitchList)
    {
        m_needStitchList[stitchIndex] = TRUE;
    }
}

ImageClassifier_Buffer::ImageClassifier_Buffer()
    : m_paneCount(0)
    , m_batchKey(-1)
{
    m_imageLot = std::make_shared<VisionImageLot>();
}

ImageClassifier_Buffer::~ImageClassifier_Buffer()
{
}

void ImageClassifier_Buffer::initialize(
    LPCTSTR trayKey, const std::map<long, BOOL>& needStitchList, const long paneCount)
{
    CSingleLock lock(&m_csBuffer, TRUE);
    m_trayKey = trayKey;
    m_needStitchList = needStitchList;
    m_paneList_ready.clear();
    m_paneList_finished.clear();
    m_paneCount = paneCount;

    for (long paneIndex = 0; paneIndex < paneCount; paneIndex++)
    {
        m_paneList_ready.push_back(paneIndex);
    }

    m_batchKey = -1;
    m_batchBarcode = _T("");
}

bool ImageClassifier_Buffer::initialize(LPCTSTR filePath, long paneIndex, long batchKey, LPCTSTR batchBarcode)
{
    CSingleLock lock(&m_csBuffer, TRUE);
    m_trayKey = _T("MANUAL");
    m_needStitchList.clear();
    m_paneList_ready.clear();
    m_paneList_finished.clear();
    m_paneList_ready.push_back(paneIndex);
    m_paneCount = 1;

    m_batchKey = batchKey;
    m_batchBarcode = batchBarcode;

    if (m_imageLot->Load(filePath, false))
    {
        return true;
    }

    ASSERT(!_T("계산이 안되었다. 무슨 일인지 Image 오픈에 실패하였다"));
    return false;
}

bool ImageClassifier_Buffer::needFOV(const InspectionAreaInfo& inspAreaInfo, const FOVID& fovID)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    CSingleLock lock(&m_csBuffer, TRUE);

    if (m_needStitchList.size() == 0)
        return false;
    if (fovID.m_trayKey != m_trayKey)
        return false;
    if (fovID.m_runtimeIndex != m_imageLot->GetInfo().m_runTimeIndex)
        return false;
    if (fovID.m_trayIndex != m_imageLot->GetInfo().m_trayIndex)
        return false;

    long stitchIndex = bIsSideVision == true ? fovID.m_stitchIndex : inspAreaInfo.getStitchIndex(fovID.m_fovIndex);
    if (stitchIndex < 0)
        return false;
    if (bIsSideVision == true)
        return true;

    if (m_needStitchList.find(stitchIndex) == m_needStitchList.end())
        return false;

    return true;
}

void ImageClassifier_Buffer::eraseNeedStitchList(long stitchIndex)
{
    CSingleLock lock(&m_csBuffer, TRUE);

    m_needStitchList.erase(stitchIndex);
}

bool ImageClassifier_Buffer::isCalculationReady()
{
    CSingleLock lock(&m_csBuffer, TRUE);
    if (m_needStitchList.size() == 0 && m_paneList_ready.size() != 0)
    {
        return true;
    }

    return false;
}

bool ImageClassifier_Buffer::isBufferCalculating()
{
    CSingleLock lock(&m_csBuffer, TRUE);

    if (m_needStitchList.size() != 0)
    {
        // 영상 수집을 다 못한 상태이다. 아직 계산도 못들어가는 상태다
        return false;
    }

    if (m_paneList_finished.size() != m_paneCount)
    {
        return true;
    }

    return false;
}

long ImageClassifier_Buffer::popCalculationPaneIndex(bool& o_isLastPaneIndex)
{
    o_isLastPaneIndex = false;

    CSingleLock lock(&m_csBuffer, TRUE);
    if (m_needStitchList.size() != 0)
    {
        return -1;
    }

    if (m_paneList_ready.size() != 0)
    {
        long paneIndex = m_paneList_ready[0];
        m_paneList_ready.erase(m_paneList_ready.begin());
        if (m_paneList_ready.size() == 0)
            o_isLastPaneIndex = true;
        return paneIndex;
    }

    return -1;
}

void ImageClassifier_Buffer::calculatationCompleted(long panelIndex)
{
    CSingleLock lock(&m_csBuffer, TRUE);

    m_paneList_finished.push_back(panelIndex);

    if (m_paneList_finished.size() == m_paneCount)
    {
        // 모든 검사 완료
        // IDLE 상태로 만든다
        setIdle();
    }
}

void ImageClassifier_Buffer::setIdle()
{
    CSingleLock lock(&m_csBuffer, TRUE);

    m_needStitchList.clear();
    m_paneList_finished.clear();
    m_paneCount = 0;
}

bool ImageClassifier_Buffer::isIdle()
{
    CSingleLock lock(&m_csBuffer, TRUE);

    return (m_needStitchList.size() == 0 && (long)m_paneList_finished.size() == m_paneCount);
}

long ImageClassifier_Buffer::getBatchKey()
{
    CSingleLock lock(&m_csBuffer, TRUE);

    return m_batchKey;
}

CString ImageClassifier_Buffer::getBatchBarcode()
{
    CSingleLock lock(&m_csBuffer, TRUE);

    return m_batchBarcode;
}

//==============================================================================================================

ImageClassifier::ImageClassifier(VisionMainTR* main, FOVImageGrabber* fovImageGrabber, long bufferCount)
    : m_fovImageGrabber(fovImageGrabber)
    , m_main(main)
{
    m_buffers.resize(bufferCount);

    for (long bufferIndex = 0; bufferIndex < (long)m_buffers.size(); bufferIndex++)
    {
        m_buffers[bufferIndex] = std::make_shared<ImageClassifier_Buffer>();
    }
}

ImageClassifier::~ImageClassifier()
{
}

void ImageClassifier::removeIncompleteImages(const VisionTrayScanSpec& trayScanSpec)
{
    CSingleLock lock(&m_csLock, TRUE);

    if (m_fovImageGrabber == nullptr)
        return;

    removeIncompleteBufferImages();
    removeIncompleteCandidates(trayScanSpec);
    removeGarbageFovImageBuffer(trayScanSpec);
}

void ImageClassifier::setAllBuffersToIdle()
{
    CSingleLock lock(&m_csLock, TRUE);

    // 후보 제거
    m_candidates.clear();

    // Buffer를 모두 IDLE 상태로 만듬
    for (long bufferIndex = 0; bufferIndex < (long)m_buffers.size(); bufferIndex++)
    {
        m_buffers[bufferIndex]->setIdle();
    }
}

void ImageClassifier::getValidBufferList(std::vector<long>& o_validList)
{
    o_validList.clear();

    CSingleLock lock(&m_csLock, TRUE);

    for (long bufferIndex = 0; bufferIndex < (long)m_buffers.size(); bufferIndex++)
    {
        if (m_buffers[bufferIndex]->isCalculationReady())
        {
            o_validList.push_back(bufferIndex);
        }
    }
}

void ImageClassifier::addCandidateDevice(std::shared_ptr<ImageClassifier_Info> candidate)
{
    CSingleLock lock(&m_csLock, TRUE);
    m_candidates.emplace_back(candidate);
}

bool ImageClassifier::update(const VisionTrayScanSpec& trayScanSpec)
{
    CSingleLock lock(&m_csLock, TRUE);

    if (m_fovImageGrabber == nullptr)
        return false;

    std::vector<long> validFovBufferIndex;
    m_fovImageGrabber->getValidBufferList(validFovBufferIndex);

    candiate_to_buffer(trayScanSpec, validFovBufferIndex);

    // Side Test - 2024.06.19_JHB
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    /////////////////////////////
    //------------------------------------------------------------------------------------------
    // 검사 이미지중 아직 붙이지 못한 FOV 이미지를 붙이자
    //------------------------------------------------------------------------------------------

    bool exitCompletedPasteImage = false;

    for (auto& fovBufferIndex : validFovBufferIndex)
    {
        auto fovInfo = m_fovImageGrabber->getBufferFovID(fovBufferIndex);

        for (auto& buffer : m_buffers)
        {
            long inspectionID = buffer->m_imageLot->GetInfo().m_scanAreaIndex;
            const auto& inspectionInfo = trayScanSpec.GetInspectionItem(inspectionID);

            if (!buffer->needFOV(inspectionInfo, fovInfo))
                continue;

            /*auto stitchIndex = inspectionInfo.getStitchIndex(fovInfo.m_fovIndex);

			m_fovImageGrabber->paste(
				fovBufferIndex,
				stitchIndex,
				inspectionInfo.m_stichCountX,
				inspectionInfo.m_stichCountY,
				buffer->m_imageLot);*/

            long stitchIndex = 0;

            // 1. SideVision이 아님
            // 2. SideVision이면서 Stitching을 하는 자재가 아닐 경우 : StitchX가 1보다 클 경우
            const bool bNeedStitch = trayScanSpec.GetMaxStitchCountX() > 1;

            if (bIsSideVision == false || (bIsSideVision == true && bNeedStitch == false))
            {
                stitchIndex = inspectionInfo.getStitchIndex(fovInfo.m_fovIndex);

                m_fovImageGrabber->paste(fovBufferIndex, stitchIndex, inspectionInfo.m_stichCountX,
                    inspectionInfo.m_stichCountY, buffer->m_imageLot);
            }
            else
            {
                stitchIndex = inspectionInfo.getStitchIndex(fovInfo.m_fovIndex);

                m_fovImageGrabber->pasteforSide(fovBufferIndex, stitchIndex, 0, inspectionInfo.m_stichCountX,
                    inspectionInfo.m_stichCountY, buffer->m_imageLot);
            }

            m_fovImageGrabber->linkDecrease(fovBufferIndex);
            exitCompletedPasteImage = true;

            buffer->eraseNeedStitchList(stitchIndex);
            if (buffer->isCalculationReady())
            {
                DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
                    _T("ImageClassifier::update Calculation Ready - I%d"), inspectionID);

                // 계산할 준비가 되었다.
                // Raw 이미지 Save 옵션을 초기화 해 놓자
                buffer->m_imageLot->InlineRawSaveCheckReset();
            }
        }
    }

    return exitCompletedPasteImage;
}

bool ImageClassifier::updateForSide(const VisionTrayScanSpec& trayScanSpec, const FOVID& fovID)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision == false)
    {
        return update(trayScanSpec);
    }

    CSingleLock lock(&m_csLock, TRUE);

    if (m_fovImageGrabber == nullptr)
        return false;

    std::vector<long> validFovBufferIndex;
    m_fovImageGrabber->getValidBufferList(validFovBufferIndex);

    candiate_to_buffer(trayScanSpec, validFovBufferIndex);

    //------------------------------------------------------------------------------------------
    // 검사 이미지중 아직 붙이지 못한 FOV 이미지를 붙이자
    //------------------------------------------------------------------------------------------

    bool exitCompletedPasteImage = false;

    for (auto& fovBufferIndex : validFovBufferIndex)
    {
        auto fovInfo = m_fovImageGrabber->getBufferFovID(fovBufferIndex);

        for (auto& buffer : m_buffers)
        {
            long inspectionID = buffer->m_imageLot->GetInfo().m_scanAreaIndex;
            const auto& inspectionInfo = trayScanSpec.GetInspectionItem(inspectionID);

            if (!buffer->needFOV(inspectionInfo, fovInfo))
                continue;

            // Fov Index 값은 Image Buffer 최대 생성 값인 64보다 같거나 작아야 하는 조건 필요
            // -->
            //////////////////////////////////////////////////////////////////////////////////
            //const long nSideFOVindex = ((fovInfo.m_fovIndex - 1) * inspectionInfo.m_stichCountX) + fovInfo.m_stitchIndex;

            auto stitchIndex = inspectionInfo.getStitchIndex(fovInfo.m_stitchIndex /*nSideFOVindex*/);
            //auto stitchIndex = fovID.m_stitchIndex;

            m_fovImageGrabber->pasteforSide(fovBufferIndex, stitchIndex, fovID.m_fovIndex, inspectionInfo.m_stichCountX,
                inspectionInfo.m_stichCountY, buffer->m_imageLot);

            m_fovImageGrabber->linkDecrease(fovBufferIndex);
            exitCompletedPasteImage = true;

            buffer->eraseNeedStitchList(stitchIndex);
            if (buffer->isCalculationReady())
            {
                DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
                    _T("ImageClassifier::update Calculation Ready - I%d"), inspectionID);

                // 계산할 준비가 되었다.
                // Raw 이미지 Save 옵션을 초기화 해 놓자
                buffer->m_imageLot->InlineRawSaveCheckReset();
            }
        }
    }

    return exitCompletedPasteImage;
}

bool ImageClassifier::isAllIdle()
{
    CSingleLock lock(&m_csLock, TRUE);
    if (m_candidates.size() != 0)
        return false; // 대기중인 후보없음

    // Buffer 가 모두 IDLE 상태
    for (long bufferIndex = 0; bufferIndex < (long)m_buffers.size(); bufferIndex++)
    {
        if (!m_buffers[bufferIndex]->isIdle())
            return false;
    }

    return true;
}

bool ImageClassifier::isAllIdleExceptCandidate()
{
    CSingleLock lock(&m_csLock, TRUE);

    // Buffer 가 모두 IDLE 상태
    for (long bufferIndex = 0; bufferIndex < (long)m_buffers.size(); bufferIndex++)
    {
        if (!m_buffers[bufferIndex]->isIdle())
            return false;
    }

    return true;
}

std::shared_ptr<ImageClassifier_Buffer> ImageClassifier::getBuffer(long bufferIndex)
{
    CSingleLock lock(&m_csLock, TRUE);

    return m_buffers[bufferIndex];
}

std::shared_ptr<ImageClassifier_Buffer> ImageClassifier::getIdleBuffer()
{
    CSingleLock lock(&m_csLock, TRUE);

    for (long grabBufferIndex = 0; grabBufferIndex < (long)m_buffers.size(); grabBufferIndex++)
    {
        if (m_buffers[grabBufferIndex]->isIdle())
        {
            return m_buffers[grabBufferIndex];
        }
    }

    return nullptr;
}

void ImageClassifier::candiate_to_buffer(
    const VisionTrayScanSpec& trayScanSpec, const std::vector<long>& validFovBufferIndex)
{
    //------------------------------------------------------------------------------------------
    // 후보 이미지중 Buffer가 부족해서 못만든 FOV Image가 있다면 만들자
    //------------------------------------------------------------------------------------------

    for (long candidateIndex = 0; candidateIndex < long(m_candidates.size()); candidateIndex++)
    {
        auto candidate = m_candidates[candidateIndex];

        long inspectionID = candidate->m_inspectionID;
        if (inspectionID < 0 || inspectionID >= (int)trayScanSpec.GetInspectionItemCount())
        {
            continue;
        }

        //-------------------------------------------------------------------------------------------------
        // 촬용된 FOV 이미지의 정보를 이용해 해당 FOV이미지를 사용하는 후보만 Buffer로 집어넣자
        // Buffer 수가 한 Tray의 Pockeet 수보다 부족한 경우, 무턱대고 집어넣으면 버퍼가 부족해 질수 있다
        //-------------------------------------------------------------------------------------------------
        auto idleBuffer = getIdleBuffer();
        if (idleBuffer == NULL)
        {
            break;
        }

        bool use = false;
        const auto& inspectionInfo = trayScanSpec.GetInspectionItem(inspectionID);

        for (auto fovBufferIndex : validFovBufferIndex)
        {
            auto fovInfo = m_fovImageGrabber->getBufferFovID(fovBufferIndex);
            auto stitchIndex
                = /*bIsSideVision ? inspectionInfo.getStitchIndex(fovInfo.m_fovIndex + candidateIndex * trayScanSpec.GetMaxStitchCountX()) : */
                inspectionInfo.getStitchIndex(fovInfo.m_fovIndex);

            if (candidate->m_needStitchList.find(stitchIndex) != candidate->m_needStitchList.end())
            {
                // 해당 FOV Index를 사용하는 Candidate 인 것이 확인되었다.
                use = true;
            }
        }

        if (!use)
            continue;

        // inlineGrab.cpp에서 Original ScanID를 Set해줬기 때문에 여기서도 visionmain의 Imagelot에서 불러와야함.. side deviceresult
        const long nScanID = m_main->GetPrimaryVisionUnit().getImageLot().GetSideScanID();
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        idleBuffer->initialize(candidate->m_trayKey, candidate->m_needStitchList, candidate->m_paneCount);
        idleBuffer->m_imageLot->SetRunTimeIndex(candidate->m_runtimeIndex);
        idleBuffer->m_imageLot->SetTrayIndex(candidate->m_trayIndex);
        idleBuffer->m_imageLot->SetScanAreaIndex(candidate->m_inspectionID);
        idleBuffer->m_imageLot->SetSideScanID(nScanID);

        static long nRepeatidx = SystemConfig::GetInstance().IsVisionTypeSide() == TRUE ? 2 : 1;
        for (long nidx = 0; nidx < nRepeatidx; nidx++)
            idleBuffer->m_imageLot->SetScales(SystemConfig::GetInstance().GetScale(enSideVisionModule(nidx)));

        auto& inspectInfo = trayScanSpec.GetInspectionItem(candidate->m_inspectionID);

        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
        {
            // 3D인 경우 이미지 Size를 재설정한다.
            // 3D는 Tray Stitch에 의해 StitchCountY가 가변이다

            idleBuffer->m_imageLot->SetRealImageSize(idleBuffer->m_imageLot->GetImageSizeX(),
                idleBuffer->m_imageLot->GetInfo().m_sensorSizeX * inspectInfo.m_stichCountY);
        }

        m_candidates.erase(m_candidates.begin() + candidateIndex);
        candidateIndex--;
    }
}

void ImageClassifier::removeIncompleteBufferImages()
{
    for (long bufferIndex = 0; bufferIndex < (long)m_buffers.size(); bufferIndex++)
    {
        if (!m_buffers[bufferIndex]->isIdle() && !m_buffers[bufferIndex]->isCalculationReady()
            && !m_buffers[bufferIndex]->isBufferCalculating())
        {
            // 이미지 수집이 완료되지 않은 버퍼는
            // 더이상의 이미지 수집을 포기하고 Idle 상태로 바꾼다

            m_buffers[bufferIndex]->setIdle();
        }
    }
}

void ImageClassifier::removeIncompleteCandidates(const VisionTrayScanSpec& trayScanSpec)
{
    // 후보들의 필요한 Stitch Index 정보를 수집한다
    std::vector<std::map<long, BOOL>> candidate_needStitchList;
    std::vector<std::vector<long>> candidate_linkFovBuffer;

    candidate_linkFovBuffer.resize(m_candidates.size());

    for (long candidateIndex = 0; candidateIndex < (long)m_candidates.size(); candidateIndex++)
    {
        auto candidate = m_candidates[candidateIndex];
        candidate_needStitchList.push_back(candidate->m_needStitchList);
    }

    std::vector<long> validBufferList;
    m_fovImageGrabber->getValidBufferList(validBufferList);

    for (auto fovBufferIndex : validBufferList)
    {
        auto fovBufferFovID = m_fovImageGrabber->getBufferFovID(fovBufferIndex);

        // 후보중 필요한 FOV 이미지가 현재 존재하는 지 확인한다
        for (long candidateIndex = 0; candidateIndex < (long)m_candidates.size(); candidateIndex++)
        {
            auto candidate = m_candidates[candidateIndex];
            if (fovBufferFovID.m_trayKey != candidate->m_trayKey)
                continue;
            if (fovBufferFovID.m_trayIndex != candidate->m_trayIndex)
                continue;
            if (fovBufferFovID.m_runtimeIndex != candidate->m_runtimeIndex)
                continue;

            const auto& inspectionInfo = trayScanSpec.GetInspectionItem(candidate->m_inspectionID);
            auto stitchIndex = inspectionInfo.getStitchIndex(fovBufferFovID.m_fovIndex);

            if (stitchIndex < 0)
                continue;

            candidate_needStitchList[candidateIndex].erase(stitchIndex);
            candidate_linkFovBuffer[candidateIndex].push_back(fovBufferIndex);
        }
    }

    // 후보중 이제 영원히 완료가 안될 후보는 제거한다
    for (long candidateIndex = 0; candidateIndex < (long)m_candidates.size(); candidateIndex++)
    {
        auto candidate = m_candidates[candidateIndex];

        if (candidate_needStitchList[candidateIndex].size() == 0)
            continue;

        // FOV Buffer의 링크를 사용안되게 된만큼 감소시킨다
        for (auto fovBufferIndex : candidate_linkFovBuffer[candidateIndex])
        {
            m_fovImageGrabber->linkDecrease(fovBufferIndex);
        }

        candidate_needStitchList.erase(candidate_needStitchList.begin() + candidateIndex);
        candidate_linkFovBuffer.erase(candidate_linkFovBuffer.begin() + candidateIndex);
        m_candidates.erase(m_candidates.begin() + candidateIndex);

        candidateIndex--;
    }
}

void ImageClassifier::removeGarbageFovImageBuffer(const VisionTrayScanSpec& trayScanSpec)
{
    std::vector<long> validFovBufferIndex;
    m_fovImageGrabber->getValidBufferList(validFovBufferIndex);

    for (auto fovBufferIndex : validFovBufferIndex)
    {
        auto fovInfo = m_fovImageGrabber->getBufferFovID(fovBufferIndex);
        bool isNeeded = false;

        for (auto buffer : m_buffers)
        {
            long inspectionID = buffer->m_imageLot->GetInfo().m_scanAreaIndex;
            const auto& inspectionInfo = trayScanSpec.GetInspectionItem(inspectionID);

            if (buffer->needFOV(inspectionInfo, fovInfo))
            {
                // Buffer중 필요한 놈이 있다
                isNeeded = true;
                break;
            }
        }

        for (long candidateIndex = 0; candidateIndex < (long)m_candidates.size(); candidateIndex++)
        {
            auto candidate = m_candidates[candidateIndex];
            auto inspectionID = candidate->m_inspectionID;

            if (candidate->m_trayKey != fovInfo.m_trayKey)
                continue;
            if (candidate->m_trayIndex != fovInfo.m_trayIndex)
                continue;
            if (candidate->m_runtimeIndex != fovInfo.m_runtimeIndex)
                continue;
            if (inspectionID < 0 || inspectionID >= trayScanSpec.GetInspectionItemCount())
                continue;

            const auto& inspectionInfo = trayScanSpec.GetInspectionItem(inspectionID);

            auto stitchIndex = inspectionInfo.getStitchIndex(fovInfo.m_fovIndex);
            if (stitchIndex < 0)
                continue;

            if (candidate->m_needStitchList.find(stitchIndex) != candidate->m_needStitchList.end())
            {
                // Buffer 후보중 필요한 놈이 있다
                isNeeded = true;
                break;
            }
        }

        if (!isNeeded)
        {
            m_fovImageGrabber->setIdleBuffer(fovBufferIndex);
        }
    }
}
