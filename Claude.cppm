module;

#include <iostream>

#include <curl/curl.h>
#include <format>
#include <string>
#include <stdexcept>

#pragma comment(lib, "libcurl.lib")

export module Claude;
export import :JSON;
export import :Constants;
export import :Objects;

using std::string;

export namespace Claude
{
    class API
    {
    private:
        //Header Strings
        curl_slist *                        m_Header;
        const string                        m_ApiHeader;;
        const string                        m_VersionHeader;
        const string                        m_ContentHeader;

        const string                        m_AuthKey;
        CURL*                               m_Context;
        CURLcode                            m_LastError;

        char*                               m_OutgoingBuffer;
        const size_t                        m_MaxOutgoingSize;
        size_t                              m_OutgoingPosition;

        char*                               m_ResponseBuffer;
        size_t                              m_ResponseBufferHead;

    protected:
        static size_t RecieveResponse(char* contents, size_t size, size_t nitems, void* stream);

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

size_t API::RecieveResponse(char* contents, size_t size, size_t nitems, void* input)
{
    size_t realsize = size * nitems;
    API * _this = (API*) input;

    _this->m_ResponseBuffer = (char *) realloc(_this->m_ResponseBuffer, _this->m_ResponseBufferHead + realsize + 1);
    if (_this->m_ResponseBuffer == NULL) {
        /* out of memory */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(_this->m_ResponseBuffer[_this->m_ResponseBufferHead]), contents, realsize);
    _this->m_ResponseBufferHead += realsize;
    _this->m_ResponseBuffer[_this->m_ResponseBufferHead] = 0;

    return realsize;
}

API::API(const std::string api_key) : m_Header(nullptr), m_MaxOutgoingSize(16 * 1024),
    m_ApiHeader(std::format("x-api-key: {0}", api_key)),
    m_VersionHeader(std::format("anthropic-version: {0}", Constants::VersionDate.data())),
    m_ContentHeader(std::format("content-type: {0}", Constants::ContentType.data()))
{
    static bool GlobalInit = (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);
    m_Context = curl_easy_init();

    if (!m_Context)
        std::runtime_error("Failed to start CURL");

    curl_easy_setopt(m_Context, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_easy_setopt(m_Context, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

    m_OutgoingBuffer = new char[m_MaxOutgoingSize];
    m_OutgoingPosition = 0;

    m_ResponseBuffer = (char *) malloc(1);
    m_ResponseBufferHead = 0;

    curl_easy_setopt(m_Context, CURLOPT_WRITEFUNCTION, API::RecieveResponse);
    curl_easy_setopt(m_Context, CURLOPT_WRITEDATA, (void*) this);

    //Initialize Header

    m_Header = curl_slist_append(m_Header, m_ApiHeader.c_str());
    m_Header = curl_slist_append(m_Header, m_VersionHeader.c_str());
    m_Header = curl_slist_append(m_Header, m_ContentHeader.c_str());
}

void API::Send(JSON::Base* t)
{
    if (!m_Context)
        std::runtime_error("CURL has not been started.");

    size_t Remaining = m_MaxOutgoingSize;
    char* BufferStart = m_OutgoingBuffer;

    m_OutgoingPosition = t->Serialize(BufferStart, Remaining);

    curl_easy_setopt(m_Context, CURLOPT_URL, "https://api.anthropic.com/v1/messages", 443);
    curl_easy_setopt(m_Context, CURLOPT_POST, 1L);

    curl_easy_setopt(m_Context, CURLOPT_HTTPHEADER, m_Header);
    curl_easy_setopt(m_Context, CURLOPT_POSTFIELDSIZE, m_OutgoingPosition);
    curl_easy_setopt(m_Context, CURLOPT_POSTFIELDS, m_OutgoingBuffer);

    m_LastError = curl_easy_perform(m_Context);
    std::cout << "Message Sent\n";

    if (m_LastError != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(m_LastError));

    std::cout << this->m_ResponseBuffer;
}

template <typename T> requires (std::is_base_of_v<JSON::Base, T>)
T* API::Create() {
    return new T();
}

