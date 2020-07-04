#include "httpuploadtest.h"
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <QFileDialog>
#include <QDebug>

#define ADR_STREAM_JID                Action::DR_StreamJid
#define ADR_CONTACT_JID               Action::DR_Parametr1
#define ADR_FILE_NAME                 Action::DR_Parametr2

HttpUploadTest::HttpUploadTest()
{
	FMessageWidgets = NULL;
	FHttpUpload = NULL;
	FMultiChatManager = NULL;
}

HttpUploadTest::~HttpUploadTest()
{
}

void HttpUploadTest::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("HTTP Upload Test");
	APluginInfo->description = tr("Test");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(HTTPUPLOAD_UUID);
}

bool HttpUploadTest::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)
	IPlugin *plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *> (plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(), SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)), SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
		}
	}
	plugin = APluginManager->pluginInterface("IMultiUserChatManager").value(0,NULL);
	if (plugin)
	{
		FMultiChatManager = qobject_cast<IMultiUserChatManager *>(plugin->instance());
	}
	plugin = APluginManager->pluginInterface("IHttpUpload").value(0,NULL);
	if (plugin)
	{
		FHttpUpload = qobject_cast<IHttpUpload *>(plugin->instance());
		if (FHttpUpload)
		{
			connect(FHttpUpload->instance(),SIGNAL(httpUploadServicesUpdated(Jid,QList<IHttpUploadService*>)),SLOT(onHttpUploadServicesUpdated(Jid,QList<IHttpUploadService*>)));
		}
	}

	return true;
}

bool HttpUploadTest::initObjects()
{
	return true;
}

bool HttpUploadTest::initSettings()
{
	return true;
}

QList <IMessageToolBarWidget *> HttpUploadTest::findToolBarWidgets(const Jid &AContactJid) const
{
	QList<IMessageToolBarWidget *> toolBars;
	foreach (IMessageToolBarWidget *widget, FToolBarActions.keys())
	{
		if (widget->messageWindow()->contactJid() == AContactJid)
			toolBars.append(widget);
	}
	return toolBars;
}

void HttpUploadTest::updateToolBarAction(IMessageToolBarWidget *AWidget)
{
	QList<IHttpUploadService*> services = FHttpUpload->availableServices(AWidget->messageWindow()->streamJid());
	qDebug() << "updateToolBarAction, Services:" << services << "streamJid:" << AWidget->messageWindow()->streamJid().bare();


	Action *uploadAction = FToolBarActions.value((AWidget));
	IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(AWidget->messageWindow()->instance());
	IMultiUserChatWindow *mucWindow = qobject_cast<IMultiUserChatWindow *>(AWidget->messageWindow()->instance());

	if (chatWindow != NULL)
	{
		if (uploadAction == NULL)
		{
			uploadAction = new Action(AWidget->toolBarChanger()->toolBar());
			uploadAction->setIcon(RSR_STORAGE_MENUICONS,MNI_HTTP_UPLOAD);
			uploadAction->setText(tr("Upload file"));
			//uploadAction->setShortcutId();
			connect(uploadAction,SIGNAL(triggered(bool)),SLOT(onUploadFileByAction(bool)));
			AWidget->toolBarChanger()->insertAction(uploadAction,TBG_MWTBW_HTTP_FILE_UPLOAD);
			FToolBarActions.insert(AWidget,uploadAction);
		}
		uploadAction->setEnabled(!services.isEmpty());
	}
	else if (mucWindow != NULL)
	{
		if (uploadAction == NULL)
		{
			uploadAction = new Action(AWidget->toolBarChanger()->toolBar());
			uploadAction->setIcon(RSR_STORAGE_MENUICONS,MNI_HTTP_UPLOAD);
			uploadAction->setText(tr("Upload file"));
			//uploadAction->setShortcutId();
			connect(uploadAction,SIGNAL(triggered(bool)),SLOT(onUploadFileByAction(bool)));
			AWidget->toolBarChanger()->insertAction(uploadAction,TBG_MWTBW_HTTP_FILE_UPLOAD);
			FToolBarActions.insert(AWidget,uploadAction);
		}
		uploadAction->setEnabled(!services.isEmpty() && mucWindow->multiUserChat()->isOpen());
	}
}

void HttpUploadTest::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
	IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(AWidget->messageWindow()->instance());
	IMultiUserChatWindow *mucWindow = qobject_cast<IMultiUserChatWindow *>(AWidget->messageWindow()->instance());
	if (chatWindow)
		connect(AWidget->messageWindow()->address()->instance(),SIGNAL(addressChanged(const Jid &,const Jid &)),SLOT(onToolbarWidgetAddressChanged(Jid,Jid)));
	if (mucWindow)
		connect(mucWindow->multiUserChat()->instance(),SIGNAL(stateChanged(int)),SLOT(onMultiUserChatStateChanged(int)));
	connect(AWidget->instance(),SIGNAL(destroyed(QObject*)),SLOT(onToolBarWidgetDestroyed(QObject*)));


	updateToolBarAction(AWidget);
}

void HttpUploadTest::onToolbarWidgetAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore); Q_UNUSED(AContactBefore);
	IMessageAddress *address = qobject_cast<IMessageAddress *>(sender());
	if (address)
	{
		foreach (IMessageToolBarWidget *widget, FToolBarActions.keys())
		{
			if (widget->messageWindow()->address() == address)
				updateToolBarAction(widget);
		}
	}
}

void HttpUploadTest::onMultiUserChatStateChanged(int AState)
{
	Q_UNUSED(AState);
	IMultiUserChat *multiChat = qobject_cast<IMultiUserChat *>(sender());
	if (multiChat)
	{
		foreach (IMessageToolBarWidget *widget, findToolBarWidgets(multiChat->roomJid()))
		{
			updateToolBarAction(widget);
		}
	}
}

void HttpUploadTest::onToolBarWidgetDestroyed(QObject *AObject)
{
	foreach (IMessageToolBarWidget *widget, FToolBarActions.keys())
	{
		if (qobject_cast<QObject *>(widget->instance()) == AObject)
			FToolBarActions.remove(widget);
	}
}

void HttpUploadTest::onHttpUploadServicesUpdated(const Jid &AStreamJid, const QList<IHttpUploadService *> &AServices)
{
	qDebug() << "HTTP Upload services updated!" << AServices;

	foreach (IMessageToolBarWidget *widget, FToolBarActions.keys())
	{
		if (widget->messageWindow()->streamJid() == AStreamJid)
			updateToolBarAction(widget);
	}
}

void HttpUploadTest::onUploadFileByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		IMessageToolBarWidget *widget = FToolBarActions.key(action);

		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid contactJid = action->data(ADR_CONTACT_JID).toString();
		QString file = action->data(ADR_FILE_NAME).toString();

		if (file.isEmpty())
		{
			QWidget *parent = widget!=NULL ? widget->messageWindow()->instance() : NULL;
			file = QFileDialog::getOpenFileName(parent,tr("Select File"));
		}

		if (!file.isEmpty())
		{
			if (streamJid.isValid() && contactJid.isValid())
				sendFile(streamJid,contactJid,file);
			else if (widget != NULL)
				sendFile(widget->messageWindow()->streamJid(),widget->messageWindow()->contactJid(),file);
		}

		/*if (!file.isEmpty())
		{
			if (streamJid.isValid() && contactJid.isValid())
				;
			else if (widget != NULL)
				;
		}*/
	}
}

void HttpUploadTest::sendFile(const Jid &AStreamJid, const Jid &AContactJid, const QString &AFileName)
{
	HttpUploadDialog *dialog = new HttpUploadDialog(AStreamJid,AFileName,FHttpUpload);

	connect(dialog,SIGNAL(dialogDestroyed()),SLOT(onHttpUploadDialogDistroyed()));
	dialog->show();
}

/*void HttpUploadTest::onHttpUploadDialogDistroyed()
{
	HttpUploadDialog *dialog = qobject_cast<HttpUploadDialog *>(sender());
	if (dialog)
	{}
}*/
