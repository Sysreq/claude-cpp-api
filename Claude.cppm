module;

#include <curl/curl.h>
#include <format>
#include <string>
#include <stdexcept>

#pragma comment(lib, "libcurl.lib")

export module Claude;
export import :Json;
export import :Constants;

using std::string;


export namespace Claude
{
    class API
    {
    private:
        const string                        m_AuthKey;
        CURL*                               m_Context;
        CURLcode                            m_LastError;

    protected:
        curl_slist* GetHeader();

    public:
        API(const std::string api_key);
        void Send(JSON::Base* t);

        template <typename T>
        requires (std::is_base_of_v<JSON::Base, T>)
        T* Create();

        ~API() {
            curl_easy_cleanup(m_Context);
            curl_global_cleanup();
        };
    };
}

using namespace Claude;

curl_slist* API::GetHeader()
{
    static const string API_KEY = std::format("x-api-key: {0}", m_AuthKey);
    static const string API_VERSION = std::format("anthropic-version: {0}", Constants::VersionDate.data());
    static const string API_CONTENT = std::format("content-type: {0}", Constants::ContentType.data());

    static struct curl_slist* headers = NULL;

    if (!headers)
    {
        headers = curl_slist_append(headers, API_KEY.c_str());
        headers = curl_slist_append(headers, API_VERSION.c_str());
        headers = curl_slist_append(headers, API_CONTENT.c_str());
    }

    return headers;
}

API::API(const std::string api_key) : m_AuthKey(api_key) 
{
    static bool GlobalInit = (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);
    m_Context = curl_easy_init();

    if (!m_Context)
        std::runtime_error("Failed to start CURL");

    curl_easy_setopt(m_Context, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_easy_setopt(m_Context, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    
    GetHeader();
}

void API::Send(JSON::Base* t)
{
    if (!m_Context)
        std::runtime_error("CURL has not been started.");

    string outbound = t->print();

    curl_easy_setopt(m_Context, CURLOPT_URL, "https://api.anthropic.com/v1/messages", 443);
    curl_easy_setopt(m_Context, CURLOPT_POST, 1L);

    curl_easy_setopt(m_Context, CURLOPT_HTTPHEADER, GetHeader());
    curl_easy_setopt(m_Context, CURLOPT_POSTFIELDS, outbound.c_str());

    m_LastError = curl_easy_perform(m_Context);

    if (m_LastError != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(m_LastError));
}

template <typename T> requires (std::is_base_of_v<JSON::Base, T>)
T* API::Create() {
    return new T();
}

