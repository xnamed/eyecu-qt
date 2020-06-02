#ifndef HTTPUPLOADTEST_H
#define HTTPUPLOADTEST_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ihttpuploadtest.h>
#include <interfaces/ihttpupload.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imultiuserchat.h>

class HttpUploadTest:
		public QObject,
		public IPlugin,
		public IHttpUploadTest
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IHttpUploadTest)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IHttpUploadTest")
#endif
public:
	HttpUploadTest();
	~HttpUploadTest();
	//IPlugin
	virtual QObject *instance() {return this;}
	virtual QUuid pluginUuid() const {return HTTPUPLOADTEST_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() {return true;}
protected:
	void updateToolBarAction(IMessageToolBarWidget *AWidget);
	QList<IMessageToolBarWidget *> findToolBarWidgets(const Jid &AContactJid) const;
protected slots:
	void onMultiUserChatStateChanged(int AState);
protected slots:
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onToolbarWidgetAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onToolBarWidgetDestroyed(QObject *AObject);
protected slots:
	void onHttpUploadServicesUpdated(const Jid &AStreamJid, const QList<IHttpUploadService *> &AServices);
protected slots:
	void onUploadFileByAction(bool);
private:
	IMessageWidgets *FMessageWidgets;
	IMultiUserChatManager *FMultiChatManager;
	IHttpUpload *FHttpUpload;
private:
	QMap<IMessageToolBarWidget *, Action *> FToolBarActions;
};

#endif // HTTPUPLOADTEST_H
