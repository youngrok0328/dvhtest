//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ParaDB.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ParaDB::ParaDB(VisionProcessing& parent)
    : m_parent(parent)
{
}

ParaDB::~ParaDB()
{
}

void ParaDB::Load()
{
    CString filePath = DynamicSystemPath::get(DefineFolder::Job_PadAlignAlgo) + _T("PadAlgoDB.db");
    CiDataBase db;
    db.Load(filePath);

    LinkDataBase(FALSE, db);

    filePath.Empty();
}

void ParaDB::Save()
{
    CString filePath = DynamicSystemPath::get(DefineFolder::Job_PadAlignAlgo) + _T("PadAlgoDB.db");
    CiDataBase db;

    LinkDataBase(TRUE, db);

    db.Save(filePath);

    filePath.Empty();
}

long ParaDB::count() const
{
    return long(m_algorithm_names.size());
}

bool ParaDB::ExistAlgorithmName(LPCTSTR name) const
{
    return m_name_to_para_index.find(name) != m_name_to_para_index.end();
}

LPCTSTR ParaDB::getAlgoritmName(long index) const
{
    return m_algorithm_names[index];
}

const AlignPara* ParaDB::getAlgorithmPara(LPCTSTR name) const
{
    auto itIndex = m_name_to_para_index.find(name);
    if (itIndex == m_name_to_para_index.end())
        return nullptr;

    return m_algorithm_paras[itIndex->second];
}

void ParaDB::add(LPCTSTR name, const AlignPara& para)
{
    auto it_paraIndex = m_name_to_para_index.find(name);
    if (it_paraIndex != m_name_to_para_index.end())
    {
        *m_algorithm_paras[it_paraIndex->second] = para;
        return;
    }

    auto* newPara = new AlignPara(m_parent);
    *newPara = para;

    m_algorithm_names.push_back(name);
    m_algorithm_paras.push_back(newPara);

    Genereat_name_to_paraIndex();
}

void ParaDB::del(long index)
{
    m_algorithm_names.erase(m_algorithm_names.begin() + index);
    m_algorithm_paras.erase(m_algorithm_paras.begin() + index);

    Genereat_name_to_paraIndex();
}

BOOL ParaDB::LinkDataBase(BOOL save, CiDataBase& db)
{
    if (!save)
    {
        m_algorithm_names.clear();
        m_algorithm_paras.clear();
        m_name_to_para_index.clear();
    }

    long count = long(m_algorithm_names.size());

    if (!db[_T("count")].Link(save, count))
        count = 0;

    if (!save)
    {
        m_algorithm_names.resize(count);

        for (auto* ptr : m_algorithm_paras)
        {
            delete ptr;
        }

        m_algorithm_paras.clear();

        for (long index = 0; index < count; index++)
        {
            m_algorithm_paras.push_back(new AlignPara(m_parent));
        }
    }

    for (long index = 0; index < count; index++)
    {
        auto& subDB = db.GetSubDBFmt(_T("item%d"), index + 1);

        subDB[_T("name")].Link(save, m_algorithm_names[index]);
        m_algorithm_paras[index]->LinkDataBase(save, subDB[_T("para")]);
    }

    if (!save)
    {
        Genereat_name_to_paraIndex();
    }
    return TRUE;
}

void ParaDB::Genereat_name_to_paraIndex()
{
    m_name_to_para_index.clear();
    for (long index = 0; index < long(m_algorithm_names.size()); index++)
    {
        m_name_to_para_index[m_algorithm_names[index]] = index;
    }
}
