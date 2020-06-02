#ifndef IHTTPUPLOADTEST_H
#define IHTTPUPLOADTEST_H

#include <QObject>

#define HTTPUPLOADTEST_UUID "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
class IHttpUploadTest
{
public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IHttpUploadTest,"RWS.Plugin.IHttpUploadTest/1.0")

#endif //IHTTPUPLOADTEST_H
