#ifndef HTTPUPLOADDIALOG_H
#define HTTPUPLOADDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <utils/jid.h>
#include <interfaces/ihttpupload.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>
#include "ui_httpuploaddialog.h"

class HttpUploadDialog:
		public QDialog
{
	Q_OBJECT
public:
	HttpUploadDialog(const Jid &AStreamJid, const QString &AFileName, IHttpUpload *AHttpUpload);
	~HttpUploadDialog();
	QString getContentType(const QString &AFileName);
	void updateServices();
signals:
	void dialogDestroyed();
protected slots:
	void onDialogButtonClicked(QAbstractButton *AButton);
	void onFileButtonClicked(bool);
	void onAccountSelected(int AIndex);
	void onServiceSelected(int AIndex);
	void onHttpUploadError(int AId, const QString &AError);
	void onHttpUploadFinished(int AId, const QUrl &AUrl);
protected:
	QString sizeName(qint64 Abytes) const;
private:
	Ui::UploadDialogClass ui;
private:
	IHttpUpload *FHttpUpload;
	QList<IHttpUploadService *> FAvailableServices;
	QString FFileUpload;
	QHash<QString, QString> FContentTypes;
	IPresenceManager *FPresenceManager;
	IAccountManager *FAccountManager;
	QHash<IHttpUploadService *, QHash<int, QFile *> > FFile;
};

#endif // HTTPUPLOADDIALOG_H
