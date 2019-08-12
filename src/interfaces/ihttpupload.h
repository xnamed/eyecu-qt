#ifndef IHTTPUPLOAD_H
#define IHTTPUPLOAD_H

#define HTTPUPLOAD_UUID "{c3854a22-432a-d901-1213-57ac94294092}"

class IHttpUpload
{
public:
        virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IHttpUpload,"RWS.Plugin.IHttpUpload/1.0")

#endif // IHTTPUPLOAD_H
