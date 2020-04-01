//
// Created by caesar on 2019/12/6.
//

#ifndef KPROXYCPP_HTTPRESPONSECODE_H
#define KPROXYCPP_HTTPRESPONSECODE_H

#include <string>

class HttpResponseCode {
public:
    enum ResponseCode {
        //	继续。客户端应继续其请求
        Continue = 100,
        //	切换协议。服务器根据客户端的请求切换协议。只能切换到更高级的协议，例如，切换到HTTP的新版本协议
        SwitchingProtocols = 101,

        //	请求成功。一般用于GET与POST请求
        OK = 200,
        //	已创建。成功请求并创建了新的资源
        Created = 201,
        //	已接受。已经接受请求，但未处理完成
        Accepted = 202,
        //	非授权信息。请求成功。但返回的meta信息不在原始的服务器，而是一个副本
        NonAuthoritativeInformation = 203,
        //	无内容。服务器成功处理，但未返回内容。在未更新网页的情况下，可确保浏览器继续显示当前文档
        NoContent = 204,
        //	重置内容。服务器处理成功，用户终端（例如：浏览器）应重置文档视图。可通过此返回码清除浏览器的表单域
        ResetContent = 205,
        //	部分内容。服务器成功处理了部分GET请求
        PartialContent = 206,

        //	多种选择。请求的资源可包括多个位置，相应可返回一个资源特征与地址的列表用于用户终端（例如：浏览器）选择
        MultipleChoices = 300,
        //	永久移动。请求的资源已被永久的移动到新URI，返回信息会包括新的URI，浏览器会自动定向到新URI。今后任何新的请求都应使用新的URI代替
        MovedPermanently = 301,
        //	临时移动。与301类似。但资源只是临时被移动。客户端应继续使用原有URI
        Found = 302,
        //	查看其它地址。与301类似。使用GET和POST请求查看
        SeeOther = 303,
        //	未修改。所请求的资源未修改，服务器返回此状态码时，不会返回任何资源。客户端通常会缓存访问过的资源，通过提供一个头信息指出客户端希望只返回在指定日期之后修改的资源
        NotModified = 304,
        //	使用代理。所请求的资源必须通过代理访问
        UseProxy = 305,
        //	已经被废弃的HTTP状态码
        Unused = 306,
        //	临时重定向。与302类似。使用GET请求重定向
        TemporaryRedirect = 307,

        //	客户端请求的语法错误，服务器无法理解
        BadRequest = 400,
        //	请求要求用户的身份认证
        Unauthorized = 401,
        //	保留，将来使用
        PaymentRequired = 402,
        //	服务器理解请求客户端的请求，但是拒绝执行此请求
        Forbidden = 403,
        //	服务器无法根据客户端的请求找到资源（网页）。通过此代码，网站设计人员可设置"您所请求的资源无法找到"的个性页面
        NotFound = 404,
        //	客户端请求中的方法被禁止
        MethodNotAllowed = 405,
        //	服务器无法根据客户端请求的内容特性完成请求
        NotAcceptable = 406,
        //	请求要求代理的身份认证，与401类似，但请求者应当使用代理进行授权
        ProxyAuthenticationRequired = 407,
        //	服务器等待客户端发送的请求时间过长，超时
        RequestTimeout = 408,
        //	服务器完成客户端的PUT请求是可能返回此代码，服务器处理请求时发生了冲突
        Conflict = 409,
        //	客户端请求的资源已经不存在。410不同于404，如果资源以前有现在被永久删除了可使用410代码，网站设计人员可通过301代码指定资源的新位置
        Gone = 410,
        //	服务器无法处理客户端发送的不带Content-Length的请求信息
        LengthRequired = 411,
        //	客户端请求信息的先决条件错误
        PreconditionFailed = 412,
        //	由于请求的实体过大，服务器无法处理，因此拒绝请求。为防止客户端的连续请求，服务器可能会关闭连接。如果只是服务器暂时无法处理，则会包含一个Retry-After的响应信息
        RequestEntityTooLarge = 413,
        //	请求的URI过长（URI通常为网址），服务器无法处理
        RequestURITooLarge = 414,
        //	服务器无法处理请求附带的媒体格式
        UnsupportedMediaType = 415,
        //	客户端请求的范围无效
        RequestedRangeNotSatisfiable = 416,
        //	服务器无法满足Expect的请求头信息
        ExpectationFailed = 417,

        //	服务器内部错误，无法完成请求
        InternalServerError = 500,
        //	服务器不支持请求的功能，无法完成请求
        NotImplemented = 501,
        //	充当网关或代理的服务器，从远端服务器接收到了一个无效的请求
        BadGateway = 502,
        //	由于超载或系统维护，服务器暂时的无法处理客户端的请求。延时的长度可包含在服务器的Retry-After头信息中
        ServiceUnavailable = 503,
        //	充当网关或代理的服务器，未及时从远端服务器获取请求
        GatewayTimeOut = 504,
        //	服务器不支持请求的HTTP协议的版本，无法完成处理
        HTTPVersionNotSupported = 505,
    };

    inline static std::string get_response_code_string(ResponseCode code) {
        switch (code) {
            //	继续。客户端应继续其请求
            case Continue:
                return "Continue";
                //	切换协议。服务器根据客户端的请求切换协议。只能切换到更高级的协议，例如，切换到HTTP的新版本协议
            case SwitchingProtocols:
                return "Switching Protocols";

                //	已创建。成功请求并创建了新的资源
            case Created:
                return "Created";
                //	已接受。已经接受请求，但未处理完成
            case Accepted:
                return "Accepted";
                //	非授权信息。请求成功。但返回的meta信息不在原始的服务器，而是一个副本
            case NonAuthoritativeInformation:
                return "NonAuthoritativeInformation";
                //	无内容。服务器成功处理，但未返回内容。在未更新网页的情况下，可确保浏览器继续显示当前文档
            case NoContent:
                return "NoContent";
                //	重置内容。服务器处理成功，用户终端（例如：浏览器）应重置文档视图。可通过此返回码清除浏览器的表单域
            case ResetContent:
                return "ResetContent";
                //	部分内容。服务器成功处理了部分GET请求
            case PartialContent:
                return "PartialContent";

                //	多种选择。请求的资源可包括多个位置，相应可返回一个资源特征与地址的列表用于用户终端（例如：浏览器）选择
            case MultipleChoices:
                return "MultipleChoices";
                //	永久移动。请求的资源已被永久的移动到新URI，返回信息会包括新的URI，浏览器会自动定向到新URI。今后任何新的请求都应使用新的URI代替
            case MovedPermanently:
                return "MovedPermanently";
                //	临时移动。与301类似。但资源只是临时被移动。客户端应继续使用原有URI
            case Found:
                return "Found";
                //	查看其它地址。与301类似。使用GET和POST请求查看
            case SeeOther:
                return "SeeOther";
                //	未修改。所请求的资源未修改，服务器返回此状态码时，不会返回任何资源。客户端通常会缓存访问过的资源，通过提供一个头信息指出客户端希望只返回在指定日期之后修改的资源
            case NotModified:
                return "NotModified";
                //	使用代理。所请求的资源必须通过代理访问
            case UseProxy:
                return "UseProxy";
                //	已经被废弃的HTTP状态码
            case Unused:
                return "Unused";
                //	临时重定向。与302类似。使用GET请求重定向
            case TemporaryRedirect:
                return "TemporaryRedirect";

                //	客户端请求的语法错误，服务器无法理解
            case BadRequest:
                return "BadRequest";
                //	请求要求用户的身份认证
            case Unauthorized:
                return "Unauthorized";
                //	保留，将来使用
            case PaymentRequired:
                return "PaymentRequired";
                //	服务器理解请求客户端的请求，但是拒绝执行此请求
            case Forbidden:
                return "Forbidden";
                //	服务器无法根据客户端的请求找到资源（网页）。通过此代码，网站设计人员可设置"您所请求的资源无法找到"的个性页面
            case NotFound:
                return "NotFound";
                //	客户端请求中的方法被禁止
            case MethodNotAllowed:
                return "MethodNotAllowed";
                //	服务器无法根据客户端请求的内容特性完成请求
            case NotAcceptable:
                return "NotAcceptable";
                //	请求要求代理的身份认证，与401类似，但请求者应当使用代理进行授权
            case ProxyAuthenticationRequired:
                return "ProxyAuthenticationRequired";
                //	服务器等待客户端发送的请求时间过长，超时
            case RequestTimeout:
                return "RequestTimeout";
                //	服务器完成客户端的PUT请求是可能返回此代码，服务器处理请求时发生了冲突
            case Conflict:
                return "Conflict";
                //	客户端请求的资源已经不存在。410不同于404，如果资源以前有现在被永久删除了可使用410代码，网站设计人员可通过301代码指定资源的新位置
            case Gone:
                return "Gone";
                //	服务器无法处理客户端发送的不带Content-Length的请求信息
            case LengthRequired:
                return "LengthRequired";
                //	客户端请求信息的先决条件错误
            case PreconditionFailed:
                return "PreconditionFailed";
                //	由于请求的实体过大，服务器无法处理，因此拒绝请求。为防止客户端的连续请求，服务器可能会关闭连接。如果只是服务器暂时无法处理，则会包含一个Retry-After的响应信息
            case RequestEntityTooLarge:
                return "RequestEntityTooLarge";
                //	请求的URI过长（URI通常为网址），服务器无法处理
            case RequestURITooLarge:
                return "RequestURITooLarge";
                //	服务器无法处理请求附带的媒体格式
            case UnsupportedMediaType:
                return "UnsupportedMediaType";
                //	客户端请求的范围无效
            case RequestedRangeNotSatisfiable:
                return "RequestedRangeNotSatisfiable";
                //	服务器无法满足Expect的请求头信息
            case ExpectationFailed:
                return "ExpectationFailed";

                //	服务器内部错误，无法完成请求
            case InternalServerError:
                return "InternalServerError";
                //	服务器不支持请求的功能，无法完成请求
            case NotImplemented:
                return "NotImplemented";
                //	充当网关或代理的服务器，从远端服务器接收到了一个无效的请求
            case BadGateway:
                return "BadGateway";
                //	由于超载或系统维护，服务器暂时的无法处理客户端的请求。延时的长度可包含在服务器的Retry-After头信息中
            case ServiceUnavailable:
                return "ServiceUnavailable";
                //	充当网关或代理的服务器，未及时从远端服务器获取请求
            case GatewayTimeOut:
                return "GatewayTimeOut";
                //	服务器不支持请求的HTTP协议的版本，无法完成处理
            case HTTPVersionNotSupported:
                return "HTTPVersionNotSupported";
                //	请求成功。一般用于GET与POST请求
            case OK:
            default:
                return "OK";
        }
    }
};

#endif //KPROXYCPP_HTTPRESPONSECODE_H
