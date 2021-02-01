#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
/*******************************************************************************
 * Project:  neb
 * @file     CJsonObject.cpp
 * @brief 
 * @author   bwarliao
 * @date:    2014-7-16
 * @note
 * Modify history:
 ******************************************************************************/

#include "CJsonObject.hpp"

#ifdef _WIN32
#define snprintf _snprintf_s
#endif

namespace neb {

    CJsonObject::CJsonObject()
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        // m_pJsonData = cJson_CreateObject();  
        m_array_iter = m_mapJsonArrayRef.end();
        m_object_iter = m_mapJsonObjectRef.end();
    }

    CJsonObject::CJsonObject(const std::string &strJson)
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        m_array_iter = m_mapJsonArrayRef.end();
        m_object_iter = m_mapJsonObjectRef.end();
        Parse(strJson);
    }

    CJsonObject::CJsonObject(const CJsonObject *pJsonObject)
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        if (pJsonObject) {
            m_array_iter = m_mapJsonArrayRef.end();
            m_object_iter = m_mapJsonObjectRef.end();
            Parse(pJsonObject->ToString());
        }
    }

    CJsonObject::CJsonObject(const CJsonObject &oJsonObject)
            : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
        m_array_iter = m_mapJsonArrayRef.end();
        m_object_iter = m_mapJsonObjectRef.end();
        Parse(oJsonObject.ToString());
    }

#if __cplusplus >= 201101L

    CJsonObject::CJsonObject(CJsonObject &&oJsonObject)
            : m_pJsonData(oJsonObject.m_pJsonData),
              m_pExternJsonDataRef(oJsonObject.m_pExternJsonDataRef),
              m_pKeyTravers(oJsonObject.m_pKeyTravers),
              mc_pError(oJsonObject.mc_pError) {
        oJsonObject.m_pJsonData = NULL;
        oJsonObject.m_pExternJsonDataRef = NULL;
        oJsonObject.m_pKeyTravers = NULL;
        oJsonObject.mc_pError = NULL;
        m_strErrMsg = std::move(oJsonObject.m_strErrMsg);
        m_mapJsonArrayRef = std::move(oJsonObject.m_mapJsonArrayRef);
        m_mapJsonObjectRef = std::move(oJsonObject.m_mapJsonObjectRef);
        m_array_iter = m_mapJsonArrayRef.end();
        m_object_iter = m_mapJsonObjectRef.end();
    }

#endif

    CJsonObject::~CJsonObject() {
        Clear();
    }

    CJsonObject &CJsonObject::operator=(const CJsonObject &oJsonObject) {
        Parse(oJsonObject.ToString().c_str());
        return (*this);
    }

#if __cplusplus >= 201101L

    CJsonObject &CJsonObject::operator=(CJsonObject &&oJsonObject) {
        m_pJsonData = oJsonObject.m_pJsonData;
        oJsonObject.m_pJsonData = NULL;
        m_pExternJsonDataRef = oJsonObject.m_pExternJsonDataRef;
        oJsonObject.m_pExternJsonDataRef = NULL;
        m_pKeyTravers = oJsonObject.m_pKeyTravers;
        oJsonObject.m_pKeyTravers = NULL;
        mc_pError = oJsonObject.mc_pError;
        oJsonObject.mc_pError = NULL;
        m_strErrMsg = std::move(oJsonObject.m_strErrMsg);
        m_mapJsonArrayRef = std::move(oJsonObject.m_mapJsonArrayRef);
        m_mapJsonObjectRef = std::move(oJsonObject.m_mapJsonObjectRef);
        m_array_iter = m_mapJsonArrayRef.end();
        m_object_iter = m_mapJsonObjectRef.end();
        return (*this);
    }

#endif

    bool CJsonObject::operator==(const CJsonObject &oJsonObject) const {
        return (this->ToString() == oJsonObject.ToString());
    }

    bool CJsonObject::AddEmptySubObject(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        m_strLastObjectKey = "";
        m_object_iter = m_mapJsonObjectRef.end();
        return (true);
    }

    bool CJsonObject::AddEmptySubArray(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

    bool CJsonObject::GetKey(std::string &strKey) {
        if (IsArray()) {
            return (false);
        }
        if (m_pKeyTravers == NULL) {
            if (m_pJsonData != NULL) {
                m_pKeyTravers = m_pJsonData;
            } else if (m_pExternJsonDataRef != NULL) {
                m_pKeyTravers = m_pExternJsonDataRef;
            }
            return (false);
        } else if (m_pKeyTravers == m_pJsonData || m_pKeyTravers == m_pExternJsonDataRef) {
            cJson *c = m_pKeyTravers->child;
            if (c) {
                strKey = c->string;
                m_pKeyTravers = c->next;
                return (true);
            } else {
                return (false);
            }
        } else {
            strKey = m_pKeyTravers->string;
            m_pKeyTravers = m_pKeyTravers->next;
            return (true);
        }
    }

    void CJsonObject::ResetTraversing() {
        if (m_pJsonData != NULL) {
            m_pKeyTravers = m_pJsonData;
        } else {
            m_pKeyTravers = m_pExternJsonDataRef;
        }
    }

    CJsonObject &CJsonObject::operator[](const std::string &strKey) {
        if (strKey == m_strLastObjectKey && m_object_iter != m_mapJsonObjectRef.end()) {
            return (*(m_object_iter->second));
        }
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
        if (iter == m_mapJsonObjectRef.end()) {
            cJson *pJsonStruct = NULL;
            if (m_pJsonData != NULL) {
                if (m_pJsonData->type == cJson_Object) {
                    pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
                }
            } else if (m_pExternJsonDataRef != NULL) {
                if (m_pExternJsonDataRef->type == cJson_Object) {
                    pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
                }
            }
            if (pJsonStruct == NULL) {
                CJsonObject *pJsonObject = new CJsonObject();
                m_mapJsonObjectRef.insert(std::pair<std::string, CJsonObject *>(strKey, pJsonObject));
                return (*pJsonObject);
            } else {
                CJsonObject *pJsonObject = new CJsonObject(pJsonStruct);
                m_mapJsonObjectRef.insert(std::pair<std::string, CJsonObject *>(strKey, pJsonObject));
                return (*pJsonObject);
            }
        } else {
            m_object_iter = iter;
            m_strLastObjectKey = strKey;
            return (*(iter->second));
        }
    }

    CJsonObject &CJsonObject::operator[](unsigned int uiWhich) {
        if (uiWhich == m_uiLastArrayIndex && m_array_iter != m_mapJsonArrayRef.end()) {
            return (*(m_array_iter->second));
        }
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(uiWhich);
#else
        auto iter = m_mapJsonArrayRef.find(uiWhich);
#endif
        if (iter == m_mapJsonArrayRef.end()) {
            cJson *pJsonStruct = NULL;
            if (m_pJsonData != NULL) {
                if (m_pJsonData->type == cJson_Array) {
                    pJsonStruct = cJson_GetArrayItem(m_pJsonData, uiWhich);
                }
            } else if (m_pExternJsonDataRef != NULL) {
                if (m_pExternJsonDataRef->type == cJson_Array) {
                    pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, uiWhich);
                }
            }
            if (pJsonStruct == NULL) {
                CJsonObject *pJsonObject = new CJsonObject();
                m_mapJsonArrayRef.insert(std::pair<unsigned int, CJsonObject *>(uiWhich, pJsonObject));
                return (*pJsonObject);
            } else {
                CJsonObject *pJsonObject = new CJsonObject(pJsonStruct);
                m_mapJsonArrayRef.insert(std::pair<unsigned int, CJsonObject *>(uiWhich, pJsonObject));
                return (*pJsonObject);
            }
        } else {
            m_uiLastArrayIndex = uiWhich;
            m_array_iter = iter;
            return (*(iter->second));
        }
    }

    std::string CJsonObject::operator()(const std::string &strKey) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        } else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            return (std::string(""));
        }
        if (pJsonStruct->type == cJson_String) {
            return (pJsonStruct->valuestring);
        } else if (pJsonStruct->type == cJson_Int) {
            char szNumber[128] = {0};
            if (pJsonStruct->sign == -1) {
                if (pJsonStruct->valueint <= (int64) INT_MAX && (int64) pJsonStruct->valueint >= (int64) INT_MIN) {
                    snprintf(szNumber, sizeof(szNumber), "%d", (int32) pJsonStruct->valueint);
                } else {
#if LLONG_MAX == LLONG_MAX
                    snprintf(szNumber, sizeof(szNumber), "%" PRId64, (int64) pJsonStruct->valueint);
#else
                    snprintf(szNumber, sizeof(szNumber), "%" PRId64, (int64)pJsonStruct->valueint);
#endif
                }
            } else {
                if ((uint64) pJsonStruct->valueint <= (uint64) UINT_MAX) {
                    snprintf(szNumber, sizeof(szNumber), "%u", (uint32) pJsonStruct->valueint);
                } else {
#if LLONG_MAX == LLONG_MAX
                    snprintf(szNumber, sizeof(szNumber), "%" PRId64, pJsonStruct->valueint);
#else
                    snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
#endif
                }
            }
            return (std::string(szNumber));
        } else if (pJsonStruct->type == cJson_Double) {
            char szNumber[128] = {0};
            if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
                snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
            } else {
                snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
            }
            return (std::string(szNumber));
        } else if (pJsonStruct->type == cJson_False) {
            return (std::string("false"));
        } else if (pJsonStruct->type == cJson_True) {
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
        } else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pExternJsonDataRef, uiWhich);
            }
        }
        if (pJsonStruct == NULL) {
            return (std::string(""));
        }
        if (pJsonStruct->type == cJson_String) {
            return (pJsonStruct->valuestring);
        } else if (pJsonStruct->type == cJson_Int) {
            char szNumber[128] = {0};
            if (pJsonStruct->sign == -1) {
                if (pJsonStruct->valueint <= (int64) INT_MAX && (int64) pJsonStruct->valueint >= (int64) INT_MIN) {
                    snprintf(szNumber, sizeof(szNumber), "%d", (int32) pJsonStruct->valueint);
                } else {
#if LLONG_MAX == LLONG_MAX
                    snprintf(szNumber, sizeof(szNumber), "%" PRId64, (int64) pJsonStruct->valueint);
#else
                    snprintf(szNumber, sizeof(szNumber), "%" PRId64, (int64)pJsonStruct->valueint);
#endif
                }
            } else {
                if ((uint64) pJsonStruct->valueint <= (uint64) UINT_MAX) {
                    snprintf(szNumber, sizeof(szNumber), "%u", (uint32) pJsonStruct->valueint);
                } else {
#if LLONG_MAX == LLONG_MAX
                    snprintf(szNumber, sizeof(szNumber), "%" PRId64, pJsonStruct->valueint);
#else
                    snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
#endif
                }
            }
            return (std::string(szNumber));
        } else if (pJsonStruct->type == cJson_Double) {
            char szNumber[128] = {0};
            if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
                snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
            } else {
                snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
            }
            return (std::string(szNumber));
        } else if (pJsonStruct->type == cJson_False) {
            return (std::string("false"));
        } else if (pJsonStruct->type == cJson_True) {
            return (std::string("true"));
        }
        return (std::string(""));
    }

    bool CJsonObject::Parse(const std::string &strJson) {
        Clear();
        m_pJsonData = cJson_Parse(strJson.c_str(), &mc_pError);
        m_pKeyTravers = m_pJsonData;
        if (m_pJsonData == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + mc_pError;
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
#if __cplusplus < 201101L
        for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); ++iter)
#else
        for (auto iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end(); ++iter)
#endif
        {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
        }
        m_mapJsonArrayRef.clear();
        m_array_iter = m_mapJsonArrayRef.end();
#if __cplusplus < 201101L
        for (std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.begin();
                    iter != m_mapJsonObjectRef.end(); ++iter)
#else
        for (auto iter = m_mapJsonObjectRef.begin(); iter != m_mapJsonObjectRef.end(); ++iter)
#endif
        {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
        }
        m_mapJsonObjectRef.clear();
        m_object_iter = m_mapJsonObjectRef.end();
    }

    bool CJsonObject::IsEmpty() const {
        if (m_pJsonData != NULL) {
            return (false);
        } else if (m_pExternJsonDataRef != NULL) {
            return (false);
        }
        return (true);
    }

    bool CJsonObject::IsArray() const {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        }

        if (pFocusData == NULL) {
            return (false);
        }

        if (pFocusData->type == cJson_Array) {
            return (true);
        } else {
            return (false);
        }
    }

    std::string CJsonObject::ToString() const {
        char *pJsonString = NULL;
        std::string strJsonData = "";
        if (m_pJsonData != NULL) {
            pJsonString = cJson_PrintUnformatted(m_pJsonData);
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else {
            return (false);
        }
    }

    bool CJsonObject::Get(const std::string &strKey, std::string &strValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Object) {
                pJsonStruct = cJson_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str(), &mc_pError);
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + mc_pError;
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        m_pKeyTravers = pFocusData;
        m_strLastObjectKey = "";
        m_object_iter = m_mapJsonObjectRef.end();
        return (true);
    }

#if __cplusplus < 201101L
    bool CJsonObject::AddWithMove(const std::string& strKey, CJsonObject& oJsonObject)
{
    cJson* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJson_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJson_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJson_GetObjectItem(pFocusData, strKey.c_str()) != NULL)
    {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJson* pJsonStruct = oJsonObject.m_pJsonData;
    oJsonObject.m_pJsonData = NULL;
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
        return(false);
    }
    cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end())
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    m_pKeyTravers = pFocusData;
    m_strLastObjectKey = "";
    m_object_iter = m_mapJsonObjectRef.end();
    return(true);
}
#else

    bool CJsonObject::Add(const std::string &strKey, CJsonObject &&oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = oJsonObject.m_pJsonData;
        oJsonObject.m_pJsonData = NULL;
        if (pJsonStruct == NULL) {
            m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
            return (false);
        }
        cJson_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        auto iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        m_pKeyTravers = pFocusData;
        m_strLastObjectKey = "";
        m_object_iter = m_mapJsonObjectRef.end();
        return (true);
    }

#endif

    bool CJsonObject::Add(const std::string &strKey, const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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

#if __cplusplus < 201101L
    bool CJsonObject::ReplaceAdd(const std::string& strKey, const CJsonObject& oJsonObject)
{
    if (KeyExist(strKey))
    {
        return(Replace(strKey, oJsonObject));
    }
    return(Add(strKey, oJsonObject));
}

bool CJsonObject::ReplaceAdd(const std::string& strKey, const std::string& strValue)
{
    if (KeyExist(strKey))
    {
        return(Replace(strKey, strValue));
    }
    return(Add(strKey, strValue));
}
#endif

    bool CJsonObject::AddNull(const std::string &strKey) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        m_pKeyTravers = pFocusData;
        m_strLastObjectKey = "";
        m_object_iter = m_mapJsonObjectRef.end();
        return (true);
    }

    bool CJsonObject::Replace(const std::string &strKey, const CJsonObject &oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str(), &mc_pError);
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + mc_pError;
            return (false);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        m_strLastObjectKey = "";
        m_object_iter = m_mapJsonObjectRef.end();
        return (true);
    }

#if __cplusplus < 201101L
    bool CJsonObject::ReplaceWithMove(const std::string& strKey, CJsonObject& oJsonObject)
{
    cJson* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJson_Object)
    {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJson* pJsonStruct = oJsonObject.m_pJsonData;
    oJsonObject.m_pJsonData = NULL;
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
        return(false);
    }
    cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL)
    {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end())
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    m_strLastObjectKey = "";
    m_object_iter = m_mapJsonObjectRef.end();
    return(true);
}
#else

    bool CJsonObject::Replace(const std::string &strKey, CJsonObject &&oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = oJsonObject.m_pJsonData;
        oJsonObject.m_pJsonData = NULL;
        if (pJsonStruct == NULL) {
            m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
            return (false);
        }
        cJson_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
        if (cJson_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
            return (false);
        }
        auto iter = m_mapJsonObjectRef.find(strKey);
        if (iter != m_mapJsonObjectRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonObjectRef.erase(iter);
        }
        m_strLastObjectKey = "";
        m_object_iter = m_mapJsonObjectRef.end();
        return (true);
    }

#endif

    bool CJsonObject::Replace(const std::string &strKey, const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
#else
        auto iter = m_mapJsonObjectRef.find(strKey);
#endif
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else {
            return (false);
        }
    }

    bool CJsonObject::Get(int iWhich, std::string &strValue) const {
        cJson *pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJson_Array) {
                pJsonStruct = cJson_GetArrayItem(m_pJsonData, iWhich);
            }
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (pJsonStruct->type == cJson_Double) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str(), &mc_pError);
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + mc_pError;
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        unsigned int uiLastIndex = (unsigned int) cJson_GetArraySize(pFocusData) - 1;
#if __cplusplus < 201101L
        for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); )
#else
        for (auto iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end();)
#endif
        {
            if (iter->first >= uiLastIndex) {
                if (iter->second != NULL) {
                    delete (iter->second);
                    iter->second = NULL;
                }
                m_mapJsonArrayRef.erase(iter++);
            } else {
                iter++;
            }
        }
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

#if __cplusplus < 201101L
    bool CJsonObject::AddWithMove(CJsonObject& oJsonObject)
{
    cJson* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJson_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJson_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJson* pJsonStruct = oJsonObject.m_pJsonData;
    oJsonObject.m_pJsonData = NULL;
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
        return(false);
    }
    int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
    cJson_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    unsigned int uiLastIndex = (unsigned int)cJson_GetArraySize(pFocusData) - 1;
    for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end(); )
    {
        if (iter->first >= uiLastIndex)
        {
            if (iter->second != NULL)
            {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        }
        else
        {
            iter++;
        }
    }
    m_uiLastArrayIndex = 0;
    m_array_iter = m_mapJsonArrayRef.end();
    return(true);
}
#else

    bool CJsonObject::Add(CJsonObject &&oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = oJsonObject.m_pJsonData;
        oJsonObject.m_pJsonData = NULL;
        if (pJsonStruct == NULL) {
            m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArray(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        unsigned int uiLastIndex = (unsigned int) cJson_GetArraySize(pFocusData) - 1;
        for (auto iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end();) {
            if (iter->first >= uiLastIndex) {
                if (iter->second != NULL) {
                    delete (iter->second);
                    iter->second = NULL;
                }
                m_mapJsonArrayRef.erase(iter++);
            } else {
                iter++;
            }
        }
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

#endif

    bool CJsonObject::Add(const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str(), &mc_pError);
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + mc_pError;
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
#if __cplusplus < 201101L
        for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); )
#else
        for (auto iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end();)
#endif
        {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        }
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

#if __cplusplus < 201101L
    bool CJsonObject::AddAsFirstWithMove(CJsonObject& oJsonObject)
{
    cJson* pFocusData = NULL;
    if (m_pJsonData != NULL)
    {
        pFocusData = m_pJsonData;
    }
    else if (m_pExternJsonDataRef != NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        m_pJsonData = cJson_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJson_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJson* pJsonStruct = oJsonObject.m_pJsonData;
    oJsonObject.m_pJsonData = NULL;
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
        return(false);
    }
    int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
    cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd)
    {
        return(false);
    }
    for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end(); )
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter++);
    }
    m_uiLastArrayIndex = 0;
    m_array_iter = m_mapJsonArrayRef.end();
    return(true);
}
#else

    bool CJsonObject::AddAsFirst(CJsonObject &&oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = oJsonObject.m_pJsonData;
        oJsonObject.m_pJsonData = NULL;
        if (pJsonStruct == NULL) {
            m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
            return (false);
        }
        int iArraySizeBeforeAdd = cJson_GetArraySize(pFocusData);
        cJson_AddItemToArrayHead(pFocusData, pJsonStruct);
        int iArraySizeAfterAdd = cJson_GetArraySize(pFocusData);
        if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
            return (false);
        }
        for (auto iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end();) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        }
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

#endif

    bool CJsonObject::AddAsFirst(const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData != NULL) {
            pFocusData = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else if (m_pExternJsonDataRef != NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        } else {
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
#if __cplusplus < 201101L
        for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
                    iter != m_mapJsonArrayRef.end(); )
#else
        for (auto iter = m_mapJsonArrayRef.begin(); iter != m_mapJsonArrayRef.end();)
#endif
        {
            if (iter->first >= (unsigned int) iWhich) {
                if (iter->second != NULL) {
                    delete (iter->second);
                    iter->second = NULL;
                }
                m_mapJsonArrayRef.erase(iter++);
            } else {
                iter++;
            }
        }
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

    bool CJsonObject::Replace(int iWhich, const CJsonObject &oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = cJson_Parse(oJsonObject.ToString().c_str(), &mc_pError);
        if (pJsonStruct == NULL) {
            m_strErrMsg = std::string("prase json string error at ") + mc_pError;
            return (false);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

#if __cplusplus < 201101L
    bool CJsonObject::ReplaceWithMove(int iWhich, CJsonObject& oJsonObject)
{
    cJson* pFocusData = NULL;
    if (m_pJsonData == NULL)
    {
        pFocusData = m_pExternJsonDataRef;
    }
    else
    {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL)
    {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJson_Array)
    {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJson* pJsonStruct = oJsonObject.m_pJsonData;
    oJsonObject.m_pJsonData = NULL;
    if (pJsonStruct == NULL)
    {
        m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
        return(false);
    }
    cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJson_GetArrayItem(pFocusData, iWhich) == NULL)
    {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end())
    {
        if (iter->second != NULL)
        {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    m_uiLastArrayIndex = 0;
    m_array_iter = m_mapJsonArrayRef.end();
    return(true);
}
#else

    bool CJsonObject::Replace(int iWhich, CJsonObject &&oJsonObject) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
        cJson *pJsonStruct = oJsonObject.m_pJsonData;
        oJsonObject.m_pJsonData = NULL;
        if (pJsonStruct == NULL) {
            m_strErrMsg = "can not move a non-independent(internal) CJsonObject from one to another.";
            return (false);
        }
        cJson_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
        if (cJson_GetArrayItem(pFocusData, iWhich) == NULL) {
            return (false);
        }
        auto iter = m_mapJsonArrayRef.find(iWhich);
        if (iter != m_mapJsonArrayRef.end()) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter);
        }
        m_uiLastArrayIndex = 0;
        m_array_iter = m_mapJsonArrayRef.end();
        return (true);
    }

#endif

    bool CJsonObject::Replace(int iWhich, const std::string &strValue) {
        cJson *pFocusData = NULL;
        if (m_pJsonData == NULL) {
            pFocusData = m_pExternJsonDataRef;
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
        } else {
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
#if __cplusplus < 201101L
        std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
#else
        auto iter = m_mapJsonArrayRef.find(iWhich);
#endif
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
#pragma clang diagnostic pop