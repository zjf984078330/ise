/****************************************************************************\
*                                                                            *
*  ISE (Iris Server Engine) Project                                          *
*  http://github.com/haoxingeng/ise                                          *
*                                                                            *
*  Copyright 2013 HaoXinGeng (haoxingeng@gmail.com)                          *
*  All rights reserved.                                                      *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
\****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// ise_http.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_HTTP_H_
#define _ISE_HTTP_H_

#include "ise_options.h"
#include "ise_classes.h"
#include "ise_sysutils.h"
#include "ise_socket.h"
#include "ise_exceptions.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// ��ǰ����

class HttpHeaderStrList;
class HttpEntityHeaderInfo;
class HttpRequestHeaderInfo;
class HttpResponseHeaderInfo;
class HttpRequest;
class HttpResponse;
class CustomHttpClient;
class HttpClient;

///////////////////////////////////////////////////////////////////////////////
// Type Definitions

// HTTP version
enum HTTP_PROTO_VER
{
	HPV_1_0,
	HPV_1_1,
};

// HTTP methods
enum HTTP_METHOD_TYPE
{
	HMT_GET,
	HMT_POST,
};

// The return value of ProcessResponseHeader()
enum HTTP_NEXT_OP
{
	HNO_REDIRECT,
	HNO_RECV_CONTENT,
	HNO_EXIT,
};

///////////////////////////////////////////////////////////////////////////////
// Constant Definitions

// Default user-agent string
const char* const ISE_DEFAULT_USER_AGENT = "Mozilla/4.0 (compatible; ISE)";
// The default http port
const int DEFAULT_HTTP_PORT = 80;

// Default Timeout defines:
const int HTTP_TCP_CONNECT_TIMEOUT         = 1000*30;     // TCP connect timeout (ms).
const int HTTP_SEND_REQ_HEADER_TIMEOUT     = 1000*30;     // Send request header timeout.
const int HTTP_SEND_REQ_CONT_BLOCK_TIMEOUT = 1000*60*2;   // Send request content block timeout.
const int HTTP_RECV_RES_HEADER_TIMEOUT     = 1000*30;     // Receive response header timeout.
const int HTTP_RECV_RES_CONT_BLOCK_TIMEOUT = 1000*60*2;   // Receive response content block timeout.
const int HTTP_SOCKET_OP_TIMEOUT           = 1000*60*10;  // Socket operation (recv/send) timeout.

// Error Codes:
const int EC_HTTP_SUCCESS                  =  0;
const int EC_HTTP_UNKNOWN_ERROR            = -1;
const int EC_HTTP_SOCKET_ERROR             = -2;
const int EC_HTTP_URL_ERROR                = -3;
const int EC_HTTP_CONTENT_LENGTH_ERROR     = -4;
const int EC_HTTP_SEND_TIMEOUT             = -5;
const int EC_HTTP_RECV_TIMEOUT             = -6;
const int EC_HTTP_CANNOT_CREATE_FILE       = -7;
const int EC_HTTP_RESPONSE_TEXT_ERROR      = -8;
const int EC_HTTP_CANNOT_RECV_CONTENT      = -9;
const int EC_HTTP_IOCP_ERROR               = -10;

///////////////////////////////////////////////////////////////////////////////
// class HttpClientOptions

struct HttpClientOptions
{
public:
	int tcpConnectTimeOut;                // TCP connect timeout (ms).
	int sendReqHeaderTimeOut;             // Send request header timeout.
	int sendReqContBlockTimeOut;          // Send request content block timeout.
	int recvResHeaderTimeOut;             // Receive response header timeout.
	int recvResContBlockTimeOut;          // Receive response content block timeout.
	int socketOpTimeOut;                  // Socket operation (recv/send) timeout.
public:
	HttpClientOptions()
	{
		tcpConnectTimeOut = HTTP_TCP_CONNECT_TIMEOUT;
		sendReqHeaderTimeOut = HTTP_SEND_REQ_HEADER_TIMEOUT;
		sendReqContBlockTimeOut = HTTP_SEND_REQ_CONT_BLOCK_TIMEOUT;
		recvResHeaderTimeOut = HTTP_RECV_RES_HEADER_TIMEOUT;
		recvResContBlockTimeOut = HTTP_RECV_RES_CONT_BLOCK_TIMEOUT;
		socketOpTimeOut = HTTP_SOCKET_OP_TIMEOUT;
	}
};

///////////////////////////////////////////////////////////////////////////////
// class HttpHeaderStrList

class HttpHeaderStrList
{
private:
	StrList items_;
	string nameValueSep_;
private:
	string makeLine(const string& name, const string& value) const;
public:
	HttpHeaderStrList();
	virtual ~HttpHeaderStrList() {}

	int add(const string& str);
	void del(int index);
	void clear();
	void addStrings(const HttpHeaderStrList& strings);
	void moveToTop(const StrList& nameList);
	int indexOfName(const string& name) const;

	int getCount() const { return items_.getCount(); }
	string getString(int index) const { return items_[index]; }
	string getText() const;
	string getName(int index) const;
	string getValue(int index) const;
	string getValue(const string& name) const;
	void setValue(const string& name, const string& value);

	string operator[] (int index) const { return getString(index); }
};

///////////////////////////////////////////////////////////////////////////////
// class HttpEntityHeaderInfo

class HttpEntityHeaderInfo
{
protected:
	HttpHeaderStrList rawHeaders_;
	HttpHeaderStrList customHeaders_;
	string cacheControl_;
	string connection_;
	string contentDisposition_;
	string contentEncoding_;
	string contentLanguage_;
	INT64 contentLength_;
	INT64 contentRangeStart_;
	INT64 contentRangeEnd_;
	INT64 contentRangeInstanceLength_;
	string contentType_;
	string contentVersion_;
	string date_;
	string expires_;
	string eTag_;
	string lastModified_;
	string pragma_;
	string transferEncoding_;
protected:
	void init();
public:
	HttpEntityHeaderInfo() { init(); }
	virtual ~HttpEntityHeaderInfo() {}

	virtual void clear();
	virtual void parseHeaders();
	virtual void buildHeaders();

	bool hasContentLength() { return contentLength_ >= 0; }
	bool hasContentRange() { return contentRangeEnd_ > 0; }
	bool hasContentRangeInstance() { return contentRangeInstanceLength_ > 0; }

	HttpHeaderStrList& getRawHeaders() { return rawHeaders_; }
	HttpHeaderStrList& getCustomHeaders() { return customHeaders_; }

	const string& getCacheControl() const { return cacheControl_; }
	const string& getConnection() const { return connection_; }
	const string& getContentDisposition() const { return contentDisposition_; }
	const string& getContentEncoding() const { return contentEncoding_; }
	const string& getContentLanguage() const { return contentLanguage_; }
	const INT64& getContentLength() const { return contentLength_; }
	const INT64& getContentRangeStart() const { return contentRangeStart_; }
	const INT64& getContentRangeEnd() const { return contentRangeEnd_; }
	const INT64& getContentRangeInstanceLength() const { return contentRangeInstanceLength_; }
	const string& getContentType() const { return contentType_; }
	const string& getContentVersion() const { return contentVersion_; }
	const string& getDate() const { return date_; }
	const string& getExpires() const { return expires_; }
	const string& getETag() const { return eTag_; }
	const string& getLastModified() const { return lastModified_; }
	const string& getPragma() const { return pragma_; }
	const string& getTransferEncoding() const { return transferEncoding_; }

	void setCustomHeaders(const HttpHeaderStrList& val) { customHeaders_ = val; }
	void setCacheControl(const string& value) { cacheControl_ = value; }
	void setConnection(const string& value) { connection_ = value; }
	void setConnection(bool keepAlive) { connection_ = (keepAlive? "keep-alive" : "close"); }
	void setContentDisposition(const string& value) { contentDisposition_ = value; }
	void setContentEncoding(const string& value) { contentEncoding_ = value; }
	void setContentLanguage(const string& value) { contentLanguage_ = value; }
	void setContentLength(INT64 value) { contentLength_ = value; }
	void setContentRangeStart(INT64 value) { contentRangeStart_ = value; }
	void setContentRangeEnd(INT64 value) { contentRangeEnd_ = value; }
	void setContentRangeInstanceLength(INT64 value) { contentRangeInstanceLength_ = value; }
	void setContentType(const string& value) { contentType_ = value; }
	void setContentVersion(const string& value) { contentVersion_ = value; }
	void setDate(const string& value) { date_ = value; }
	void setExpires(const string& value) { expires_ = value; }
	void setETag(const string& value) { eTag_ = value; }
	void setLastModified(const string& value) { lastModified_ = value; }
	void setPragma(const string& value) { pragma_ = value; }
	void setTransferEncoding(const string& value) { transferEncoding_ = value; }
};

///////////////////////////////////////////////////////////////////////////////
// class HttpRequestHeaderInfo

class HttpRequestHeaderInfo : public HttpEntityHeaderInfo
{
protected:
	string accept_;
	string acceptCharSet_;
	string acceptEncoding_;
	string acceptLanguage_;
	string from_;
	string referer_;
	string userAgent_;
	string host_;
	string range_;
protected:
	void init();
public:
	HttpRequestHeaderInfo() { init(); }

	virtual void clear();
	virtual void parseHeaders();
	virtual void buildHeaders();

	const string& getAccept() const { return  accept_; }
	const string& getAcceptCharSet() const { return  acceptCharSet_; }
	const string& getAcceptEncoding() const { return  acceptEncoding_; }
	const string& getAcceptLanguage() const { return  acceptLanguage_; }
	const string& getFrom() const { return  from_; }
	const string& getReferer() const { return  referer_; }
	const string& getUserAgent() const { return  userAgent_; }
	const string& getHost() const { return  host_; }
	const string& getRange() const { return  range_; }

	void setAccept(const string& value) { accept_ = value; }
	void setAcceptCharSet(const string& value) { acceptCharSet_ = value; }
	void setAcceptEncoding(const string& value) { acceptEncoding_ = value; }
	void setAcceptLanguage(const string& value) { acceptLanguage_ = value; }
	void setFrom(const string& value) { from_ = value; }
	void setReferer(const string& value) { referer_ = value; }
	void setUserAgent(const string& value) { userAgent_ = value; }
	void setHost(const string& value) { host_ = value; }
	void setRange(const string& value) { range_ = value; }
	void setRange(INT64 rangeStart, INT64 rangeEnd = -1);
};

///////////////////////////////////////////////////////////////////////////////
// class HttpResponseHeaderInfo

class HttpResponseHeaderInfo : public HttpEntityHeaderInfo
{
protected:
	string acceptRanges_;
	string location_;
	string server_;
protected:
	void init();
public:
	HttpResponseHeaderInfo() { init(); }

	virtual void clear();
	virtual void parseHeaders();
	virtual void buildHeaders();

	const string& getAcceptRanges() const { return  acceptRanges_; }
	const string& getLocation() const { return  location_; }
	const string& getServer() const { return  server_; }

	void setAcceptRanges(const string& value) { acceptRanges_ = value; }
	void setLocation(const string& value) { location_ = value; }
	void setServer(const string& value) { server_ = value; }
};

///////////////////////////////////////////////////////////////////////////////
// class HttpRequest

class HttpRequest : public HttpRequestHeaderInfo
{
protected:
	CustomHttpClient& httpClient_;
	HTTP_PROTO_VER protocolVersion_;
	string url_;
	string method_;
	Stream *contentStream_;
protected:
	void init();
public:
	HttpRequest(CustomHttpClient& HttpClient);

	virtual void clear();

	HTTP_PROTO_VER getProtocolVersion() const { return protocolVersion_; }
	const string& getUrl() const { return url_; }
	const string& getMethod() const { return method_; }
	Stream* getContentStream() const { return contentStream_; }

	void setProtocolVersion(HTTP_PROTO_VER value) { protocolVersion_ = value; }
	void setUrl(const string& value) { url_ = value; }
	void setMethod(const string& value) { method_ = value; }
	void setContentStream(Stream *value) { contentStream_ = value; }
};

///////////////////////////////////////////////////////////////////////////////
// class HttpResponse

class HttpResponse : public HttpResponseHeaderInfo
{
protected:
	CustomHttpClient& httpClient_;
	string responseText_;
	Stream *contentStream_;
protected:
	void init();
public:
	HttpResponse(CustomHttpClient& httpClient);

	virtual void clear();

	bool getKeepAlive();
	const string& getResponseText() const { return responseText_; }
	int getResponseCode() const;
	HTTP_PROTO_VER getResponseVersion() const;
	Stream* getContentStream() const { return contentStream_; }

	void setResponseText(const string& value) { responseText_ = value; }
	void setContentStream(Stream *value) { contentStream_ = value; }
};

///////////////////////////////////////////////////////////////////////////////
// class CustomHttpClient - HTTP client base class.

class CustomHttpClient
{
public:
	friend class HttpRequest;
	friend class HttpResponse;
protected:
	TcpClient tcpClient_;
	HttpRequest request_;
	HttpResponse response_;
	HttpClientOptions options_;
	Url url_;
	bool handleRedirects_;
	int redirectCount_;
	bool lastKeepAlive_;
protected:
	InetAddress getInetAddrFromUrl(Url& url);
	void makeRequestBuffer(Buffer& buffer);
	int beforeRequest(HTTP_METHOD_TYPE httpMethod, const string& urlStr, Stream *requestContent,
		Stream *responseContent, INT64 reqStreamPos, INT64 resStreamPos);
	void checkResponseHeader(char *buffer, int size, bool& finished, bool& error);
	bool parseResponseHeader(void *buffer, int size);
	HTTP_NEXT_OP processResponseHeader();
	void tcpDisconnect(bool force = false);
public:
	CustomHttpClient();
	virtual ~CustomHttpClient();

	/// Force to disconnect the connection.
	void disconnect() { tcpDisconnect(true); }
	/// Indicates whether the connection is currently connected or not.
	bool isConnected() { return tcpClient_.isConnected(); }

	/// The http request info.
	HttpRequest& httpRequest() { return request_; }
	/// The http response info.
	HttpResponse& httpResponse() { return response_; }
	/// The http client options
	HttpClientOptions& options() { return options_; }
	/// Returns the response text.
	string getResponseText() { return response_.getResponseText(); }
	/// Returns the response code.
	int getResponseCode() { return response_.getResponseCode(); }
	/// Indicates if the http client can handle redirections.
	bool getHandleRedirects() { return handleRedirects_; }
	/// Indicates the number of redirects encountered in the last request for the http client.
	int getRedirectCount() { return redirectCount_; }
	/// Returns the tcp client object.
	TcpClient& getTcpClient() { return tcpClient_; }

	/// Determines if the http client can handle redirections.
	void setHandleRedirects(bool value) { handleRedirects_ = value; }
};

///////////////////////////////////////////////////////////////////////////////
// class HttpClient - HTTP client class.

class HttpClient : public CustomHttpClient
{
public:
	friend class AutoFinalizer;
private:
	int readLine(string& line, int timeout);
	int readChunkSize(UINT& chunkSize, int timeout);
	int readStream(Stream& stream, int bytes, int timeout);
protected:
	int executeHttpAction(HTTP_METHOD_TYPE httpMethod, const string& url,
		Stream *requestContent, Stream *responseContent);
	int executeHttpRequest(HTTP_METHOD_TYPE httpMethod, const string& url,
		Stream *requestContent, Stream *responseContent,
		bool needRecvContent, bool& canRecvContent);

	int tcpConnect();
	int sendRequestHeader();
	int sendRequestContent();
	int recvResponseHeader();
	int recvResponseContent();
public:
	HttpClient();
	virtual ~HttpClient();

	/// Sends a "GET" request to http server, and receives the response content. Returns the error code (EC_HTTP_XXX).
	int get(const string& url, Stream *responseContent);
	/// Sends a "POST" request to http server with the specified request content, and receives the response content. Returns the error code (EC_HTTP_XXX).
	int post(const string& url, Stream *requestContent, Stream *responseContent);

	/// Downloads the entire file from the specified url. Returns the error code (EC_HTTP_XXX).
	int downloadFile(const string& url, const string& localFileName);
	/// Sends the "GET" request to http server, and receives the response text and headers. Returns the error code (EC_HTTP_XXX).
	int requestFile(const string& url);
	/// Try to receive the response content from http server, returns the total number of bytes received actually, -1 if error.
	int receiveFile(void *buffer, int size, int timeoutMSecs = -1);
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_HTTP_H_
