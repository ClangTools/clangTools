/*******************************************************************************
 * Project:  neb
 * @file     CJsonObject.cpp
 * @brief 
 * @author   bwarliao
 * @date:    2014-7-16
 * @note
 * Modify history:
 ******************************************************************************/

#include "CJsonObject.h"

#ifdef _WIN32
#define snprintf _snprintf_s
#endif

namespace neb {

    CJsonObject::CJsonObject()
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        // m_pJsonData = cJson_CreateObject();
    }

    CJsonObject::CJsonObject(const std::string &strJson)
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        Parse(strJson);
    }

    CJsonObject::CJsonObject(const CJsonObject *pJsonObject)
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        if (pJsonObject) {
            Parse(pJsonObject->ToString());
        }
    }

    CJsonObject::CJsonObject(const CJsonObject &oJsonObject)
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        Parse(oJsonObject.ToString());
    }

    CJsonObject::~CJsonObject() {
        Clear();
    }

    CJsonObject &CJsonObject::operator=(const CJsonObject &oJsonObject) {
        Parse(oJsonObject.ToString().c_str());
        return (*this);
    }

    bool CJsonObject::operator==(const CJsonObject &oJsonObject) const {
        return (this->ToString() == oJsonObject.ToString());
    }

    bool CJsonObject::AddEmptySubObject(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateObject();
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("create sub empty object error!");
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::AddEmptySubArray(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateArray();
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("create sub empty array error!");
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::GetKey(std::string &strKey) {
        if (IsArray()) {
            return (false);
        }
        if (m_pKeyTravers == NULL) {
            if (m_pJsonData != NULL) {
                m_pKeyTravers = m_pJsonData;
            }
            else if (m_pExternJsonDataRef != NULL) {
                m_pKeyTravers = m_pExternJsonDataRef;
            }
            return (false);
        }
        else if (m_pKeyTravers == m_pJsonData || m_pKeyTravers == m_pExternJsonDataRef) {
            cJson *c = m_pKeyTravers->child;
            if (c) {
                strKey = c->string;
                m_pKeyTravers = c->next;
                return (true);
            }
            else {
                return (false);
            }
        }
        else {
            strKey = m_pKeyTravers->string;
            m_pKeyTravers = m_pKeyTravers->next;
            return (true);
        }
    }

    void CJsonObject::ResetTraversing() {
        if (m_pJsonData != NULL) {
            m_pKeyTravers = m_pJsonData;
        }
        else {
            m_pKeyTravers = m_pExternJsonDataRef;
        }
    }

    CJsonObject &CJsonObject::operator[](const std::string &strKey) {
        std::map<std::string, CJsonObject *>::iterator iter;
        iter = m_mapJsonObjectRef.find(strKey);
        if (iter == m_mapJsonObjectRef.end()) {
            cJson *pJsonStruct = NULL;
            if (m_pJsonData != NULL) {
                if (m_pJsonData->type == cJson_Object) {
                    pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
                }
            }
            else if (m_pExternJsonDataRef != NULL) {
                if (m_pExternJsonDataRef->type == cJson_Object) {
                    pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
                }
            }
            if (pJsonStruct == NULL) {
                CJsonObject *pJsonObject = new CJsonObject();
                m_mapJsonObjectRef.insert(std::pair<std::string, CJsonObject *>(strKey, pJsonObject));
                return (*pJsonObject);
            }
            else {
                CJsonObject *pJsonObject = new CJsonObject(pJsonStruct);
                m_mapJsonObjectRef.insert(std::pair<std::string, CJsonObject *>(strKey, pJsonObject));
                return (*pJsonObject);
            }
        }
        else {
            return (*(iter->second));
        }
    }

    CJsonObject &CJsonObject::operator[](unsigned int uiWhich) {
        std::map<unsigned int, CJsonObject *>::iterator iter;
        iter = m_mapJsonArrayRef.find(uiWhich);
        if (iter == m_mapJsonArrayRef.end()) {
            cJson *pJsonStruct = NULL;
            if (m_pJsonData != NULL) {
                if (m_pJsonData->type == cJson_Array) {
                    pJsonStruct = cJson_GetArrayItem(m_pJsonData, uiWhich);
                }
            }
            else if (m_pExternJsonDataRef != NULL) {
                if (m_pExternJsonDataRef->type == cJson_Array) {
                    pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, uiWhich);
                }
            }
            if (pJsonStruct == NULL) {
                CJsonObject *pJsonObject = new CJsonObject();
                m_mapJsonArrayRef.insert(std::pair<unsigned int, CJsonObject *>(uiWhich, pJsonObject));
                return (*pJsonObject);
            }
            else {
                CJsonObject *pJsonObject = new CJsonObject(pJsonStruct);
                m_mapJsonArrayRef.insert(std::pair<unsigned int, CJsonObject *>(uiWhich, pJsonObject));
                return (*pJsonObject);
            }
        }
        else {
            return (*(iter->second));
        }
    }

    std::string CJsonObject::operator()(const std::string &strKey) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (std::string(""));
        }
        if (pJsonStruct->type == cJson_String) {
            return (pJsonStruct->valuestring);
        }
        else if (pJsonStruct->type == cJson_Int) {
            char szNumber[128] = {0};
            if (pJsonStruct->sign == -1) {
                if (pJsonStruct->valueint <= (int64) INT_MAX && (int64) pJsonStruct->valueint >= (int64) INT_MIN) {
                    snprintf(szNumber, sizeof(szNumber), "%d", (int32) pJsonStruct->valueint);
                }
                else {
                    snprintf(szNumber, sizeof(szNumber),
#ifdef __APPLE__ // DARWIN
                             "%lld"
#else
                            "%ld"
#endif
                            , (int64) pJsonStruct->valueint);
                }
            }
            else {
                if ((uint64) pJsonStruct->valueint <= (uint64) UINT_MAX) {
                    snprintf(szNumber, sizeof(szNumber), "%u", (uint32) pJsonStruct->valueint);
                }
                else {
                    snprintf(szNumber, sizeof(szNumber),
#ifdef __APPLE__ // DARWIN
                             "%lld"
#else
                            "%ld"
#endif
                            , pJsonStruct->valueint);
                }
            }
            return (std::string(szNumber));
        }
        else if (pJsonStruct->type == cJson_Double) {
            char szNumber[128] = {0};
            if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
                snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
            }
            else {
                snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
            }
            return (std::string(szNumber));
        }
        else if (pJsonStruct->type == cJson_False) {
            return (std::string("false"));
        }
        else if (pJsonStruct->type == cJson_True) {
            return (std::string("true"));
        }
        return (std::string(""));
    }

    std::string CJsonObject::operator()(unsigned int uiWhich) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, uiWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, uiWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (std::string(""));
        }
        if (pJsonStruct->type == cJson_String) {
            return (pJsonStruct->valuestring);
        }
        else if (pJsonStruct->type == cJson_Int) {
            char szNumber[128] = {0};
            if (pJsonStruct->sign == -1) {
                if (pJsonStruct->valueint <= (int64) INT_MAX && (int64) pJsonStruct->valueint >= (int64) INT_MIN) {
                    snprintf(szNumber, sizeof(szNumber), "%d", (int32) pJsonStruct->valueint);
                }
                else {
                    snprintf(szNumber, sizeof(szNumber),
#ifdef __APPLE__ // DARWIN
                             "%lld"
#else
                            "%ld"
#endif
                            , (int64) pJsonStruct->valueint);
                }
            }
            else {
                if ((uint64) pJsonStruct->valueint <= (uint64) UINT_MAX) {
                    snprintf(szNumber, sizeof(szNumber), "%u", (uint32) pJsonStruct->valueint);
                }
                else {
                    snprintf(szNumber, sizeof(szNumber),
#ifdef __APPLE__ // DARWIN
                             "%lld"
#else
                             "%ld"
#endif
                             , pJsonStruct->valueint);
                }
            }
            return (std::string(szNumber));
        }
        else if (pJsonStruct->type == cJson_Double) {
            char szNumber[128] = {0};
            if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
                snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
            }
            else {
                snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
            }
            return (std::string(szNumber));
        }
        else if (pJsonStruct->type == cJson_False) {
            return (std::string("false"));
        }
        else if (pJsonStruct->type == cJson_True) {
            return (std::string("true"));
        }
        return (std::string(""));
    }

    bool CJsonObject::Parse(const std::string &strJson) {
        Clear();
        m_pJsonData = cJson_Parse(strJson.c_str());
        m_pKeyTravers = m_pJsonData;
        if (m_pJsonData == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + cJson_GetErrorPtr();
            return (false);
        }
        return (true);
    }

    void CJsonObject::Clear() {
        m_pExternJsonDataRef = NULL;
        m_pKeyTravers = NULL;
        if (m_pJsonData != NULL) {
            cJson_Delete(m_pJsonData);
            m_pJsonData = NULL;
        }
        for (std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.begin();
             iter != m_mapJsonArrayRef.end(); ++iter) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
        }
        m_mapJsonArrayRef.clear();
        for (std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.begin();
             iter != m_mapJsonObjectRef.end(); ++iter) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
        }
        m_mapJsonObjectRef.clear();
    }

    bool CJsonObject::IsEmpty() const {
        if (m_pJsonData != NULL) {
            return (false);
        }
        else if (m_pExternJsonDataRef != NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::IsArray() const {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }

        if (pFocusData == NULL) {
            return (false);
        }

        if (pFocusData->type == cJson_Array) {
            return (true);
        }
        else {
            return (false);
        }
    }

    std::string CJsonObject::ToString() const {
        char *pJsonString = NULL;
        std::string strJsonData = "";
        if (m_pJsonData != NULL) {
            pJsonString = cJson_PrintUnformatted(m_pJsonData);
        }
        else if (m_pExternJsonDataRef != NULL) {
            pJsonString = cJson_PrintUnformatted(m_pExternJsonDataRef);
        }
        if (pJsonString != NULL) {
            strJsonData = pJsonString;
            free(pJsonString);
        }
        return (strJsonData);
    }

    std::string CJsonObject::ToFormattedString() const {
        char *pJsonString = NULL;
        std::string strJsonData = "";
        if (m_pJsonData != NULL) {
            pJsonString = cJson_Print(m_pJsonData);
        }
        else if (m_pExternJsonDataRef != NULL) {
            pJsonString = cJson_Print(m_pExternJsonDataRef);
        }
        if (pJsonString != NULL) {
            strJsonData = pJsonString;
            free(pJsonString);
        }
        return (strJsonData);
    }

    bool CJsonObject::KeyExist(const std::string &strKey) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Get(const std::string &strKey, CJsonObject &oJsonObject) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        char *pJsonString = cJson_Print(pJsonStruct);
        std::string strJsonData = pJsonString;
        free(pJsonString);
        if (oJsonObject.Parse(strJsonData)) {
            return (true);
        }
        else {
            return (false);
        }
    }

    bool CJsonObject::Get(const std::string &strKey, std::string &strValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type != cJson_String) {
            return (false);
        }
        strValue = pJsonStruct->valuestring;
        return (true);
    }

    bool CJsonObject::Get(const std::string &strKey, int32 &iValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            iValue = (int32) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            iValue = (int32) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(const std::string &strKey, uint32 &uiValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            uiValue = (uint32) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            uiValue = (uint32) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(const std::string &strKey, int64 &llValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            llValue = (int64) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            llValue = (int64) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(const std::string &strKey, uint64 &ullValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            ullValue = (uint64) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            ullValue = (uint64) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(const std::string &strKey, bool &bValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type > cJson_True) {
            return (false);
        }
        bValue = pJsonStruct->type;
        return (true);
    }

    bool CJsonObject::Get(const std::string &strKey, float &fValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Double || pJsonStruct->type == cJson_Int) {
            fValue = (float) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(const std::string &strKey, double &dValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Double || pJsonStruct->type == cJson_Int) {
            dValue = pJsonStruct->valuedouble;
            return (true);
        }
        return (false);
    }

    bool CJsonObject::IsNull(const std::string &strKey) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type != cJson_NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, const CJsonObject &oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str());
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + cJson_GetErrorPtr();
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateString(strValue.c_str());
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, int32 iValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) iValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, uint32 uiValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) uiValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, int64 llValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) llValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, uint64 ullValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt(ullValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, bool bValue, bool bValueAgain) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateBool(bValue);
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, float fValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) fValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Add(const std::string &strKey, double dValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) dValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::ReplaceAdd(const std::string &strKey, const CJsonObject &oJsonObject) {
        if (Replace(strKey, oJsonObject) == false)
            return Add(strKey, oJsonObject);
        return true;
    }

    bool CJsonObject::ReplaceAdd(const std::string &strKey, const std::string &strValue) {
        if (Replace(strKey, strValue) == false)
            return Add(strKey, strValue);
        return true;
    }

    bool CJsonObject::AddNull(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateObject();
            m_pKeyTravers = m_pJsonData;
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
            m_strErrMsg = "key exists!";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateNull();
        if (pJsonStruct == NULL) {
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Delete(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson_DeleteItemFromObject(pFocusData, strKey.c_str());
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        m_pKeyTravers = pFocusData;
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, const CJsonObject &oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str());
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + cJson_GetErrorPtr();
            return (false);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateString(strValue.c_str());
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, int32 iValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) iValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, uint32 uiValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) uiValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, int64 llValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) llValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, uint64 ullValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) ullValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, bool bValue, bool bValueAgain) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateBool(bValue);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, float fValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) fValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, double dValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) dValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::ReplaceWithNull(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Object) {
            m_strErrMsg = "not a json object! json array?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateNull();
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<std::string, CJsonObject *>::iterator iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        return (true);
    }

    int CJsonObject::GetArraySize() {
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                return (cJson_GetArraySize(m_pJsonData));
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                return (cJson_GetArraySize(m_pExternJsonDataRef));
            }
        }
        return (0);
    }

    bool CJsonObject::Get(int iWhich, CJsonObject &oJsonObject) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        char *pJsonString = cJson_Print(pJsonStruct);
        std::string strJsonData = pJsonString;
        free(pJsonString);
        if (oJsonObject.Parse(strJsonData)) {
            return (true);
        }
        else {
            return (false);
        }
    }

    bool CJsonObject::Get(int iWhich, std::string &strValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type != cJson_String) {
            return (false);
        }
        strValue = pJsonStruct->valuestring;
        return (true);
    }

    bool CJsonObject::Get(int iWhich, int32 &iValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            iValue = (int32) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            iValue = (int32) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(int iWhich, uint32 &uiValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            uiValue = (uint32) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            uiValue = (uint32) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(int iWhich, int64 &llValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            llValue = (int64) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            llValue = (int64) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(int iWhich, uint64 &ullValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Int) {
            ullValue = (uint64) (pJsonStruct->valueint);
            return (true);
        }
        else if (pJsonStruct->type == cJson_Double) {
            ullValue = (uint64) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(int iWhich, bool &bValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type > cJson_True) {
            return (false);
        }
        bValue = pJsonStruct->type;
        return (true);
    }

    bool CJsonObject::Get(int iWhich, float &fValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Double || pJsonStruct->type == cJson_Int) {
            fValue = (float) (pJsonStruct->valuedouble);
            return (true);
        }
        return (false);
    }

    bool CJsonObject::Get(int iWhich, double &dValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type == cJson_Double || pJsonStruct->type == cJson_Int) {
            dValue = pJsonStruct->valuedouble;
            return (true);
        }
        return (false);
    }

    bool CJsonObject::IsNull(int iWhich) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        }
        else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, iWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (false);
        }
        if (pJsonStruct->type != cJson_NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(const CJsonObject &oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str());
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + cJson_GetErrorPtr();
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        unsigned int uiLastIndex = (unsigned int) cJson_GetArraySize(pFocusData) - 1;
        for (std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.begin();
             iter != m_mapJsonArrayRef.end();) {
            if (iter->first >= uiLastIndex) {
                if (iter->second != NULL) {
                    delete (iter->second);
                    iter->second = NULL;
                }
                m_mapJsonArrayRef.erase(iter++);
            }
            else {
                iter++;
            }
        }
        return (true);
    }

    bool CJsonObject::Add(const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateString(strValue.c_str());
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(int32 iValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) iValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(uint32 uiValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) uiValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(int64 llValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) llValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(uint64 ullValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) ullValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(int iAnywhere, bool bValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateBool(bValue);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(float fValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) fValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Add(double dValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) dValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddNull() {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateNull();
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(const CJsonObject &oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str());
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + cJson_GetErrorPtr();
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        for (std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.begin();
             iter != m_mapJsonArrayRef.end();) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateString(strValue.c_str());
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(int32 iValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) iValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(uint32 uiValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) uiValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(int64 llValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) llValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(uint64 ullValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) ullValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(int iAnywhere, bool bValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateBool(bValue);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(float fValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) fValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddAsFirst(double dValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) dValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::AddNullAsFirst() {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        }
        else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            m_pJsonData = cJson_CreateArray();
            pFocusData = m_pJsonData;
        }

        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateNull();
        if (pJsonStruct == NULL) {
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Delete(int iWhich) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson_DeleteItemFromArray(pFocusData, iWhich);
        for (std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.begin();
             iter != m_mapJsonArrayRef.end();) {
            if (iter->first >= (unsigned int) iWhich) {
                if (iter->second != NULL) {
                    delete (iter->second);
                    iter->second = NULL;
                }
                m_mapJsonArrayRef.erase(iter++);
            }
            else {
                iter++;
            }
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, const CJsonObject &oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str());
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + cJson_GetErrorPtr();
            return (false);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateString(strValue.c_str());
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, int32 iValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) iValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, uint32 uiValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) uiValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, int64 llValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) ((uint64) llValue), -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, uint64 ullValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateInt((uint64) ullValue, 1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, bool bValue, bool bValueAgain) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateBool(bValue);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, float fValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) fValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, double dValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateDouble((double) dValue, -1);
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::ReplaceWithNull(int iWhich) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        }
        else {
            pFocusData = m_pJsonData;
        }
        if (pFocusData == NULL) {
            m_strErrMsg = "json data is null!";
            return (false);
        }
        if (pFocusData->type != cJson_Array) {
            m_strErrMsg = "not a json array! json object?";
            return (false);
        }
        cJson *pJsonStruct = cJson_CreateNull();
        if (pJsonStruct == NULL) {
            return (false);
        }
        std::map<unsigned int, CJsonObject *>::iterator iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        return (true);
    }

    CJsonObject::CJsonObject(cJson *pJsonData)
            : m_pJsonData(NULL), m_pExternJsonDataRef(pJsonData), m_pKeyTravers(pJsonData) {
    }

}


