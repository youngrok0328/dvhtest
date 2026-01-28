#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "AlignPara.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ParaDB
{
public:
    ParaDB(const ParaDB& object) = delete;
    ParaDB(VisionProcessing& parent);
    ~ParaDB();

    void Load();
    void Save();
    BOOL LinkDataBase(BOOL save, CiDataBase& db);

    long count() const;
    bool ExistAlgorithmName(LPCTSTR name) const;
    LPCTSTR getAlgoritmName(long index) const;
    const AlignPara* getAlgorithmPara(LPCTSTR name) const;
    void add(LPCTSTR name, const AlignPara& para);
    void del(long index);

private:
    VisionProcessing& m_parent;
    std::vector<CString> m_algorithm_names;
    std::vector<AlignPara*> m_algorithm_paras;
    std::map<CString, long> m_name_to_para_index; // 자동 생성

    void Genereat_name_to_paraIndex();
};
