#include "httpuploaddialog.h"

#include <QFileDialog>
#include <utils/pluginhelper.h>
#include <QDebug>

#define DEFAULT_CONTENT_TYPE      "application/octet-stream"

HttpUploadDialog::HttpUploadDialog(const Jid &AStreamJid, const QString &AFileName, IHttpUpload *AHttpUpload):
	FFileUpload(AFileName),
	FHttpUpload(AHttpUpload),
	FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>())
{
	ui.setupUi(this);

	ui.pgbPrgress->setMaximum(0);
	ui.pgbPrgress->setMaximum(100);

	setWindowTitle(tr("Upload File - %1").arg(AStreamJid.uFull()));

	QList<IPresence*> presences = FPresenceManager->presences();
	for (QList<IPresence*>::ConstIterator it=presences.constBegin(); it!=presences.constEnd(); ++it)
	{
		IAccount *account = FAccountManager->findAccountByStream((*it)->streamJid());
		QList<IHttpUploadService*> services = FHttpUpload->availableServices((*it)->streamJid());
		if (!services.isEmpty())
		{
			ui.cmbAccount->addItem(account->name(), account->accountJid().full());
			if (account->accountJid() == AStreamJid)
			{
				ui.cmbAccount->setCurrentIndex(ui.cmbAccount->findData(account->accountJid().full()));

			}

			onAccountSelected(ui.cmbAccount->findData(account->accountJid().full()));
		}
	}

	FContentTypes[".avi"]  = "video/avi";
	FContentTypes[".bmp"]  = "image/bmp";
	FContentTypes[".bz2"]  = "application/x-bzip2";
	FContentTypes[".gif"]  = "image/gif";
	FContentTypes[".gz"]   = "application/x-gzip";
	FContentTypes[".jpeg"] = "image/jpeg";
	FContentTypes[".jpg"]  = "image/jpeg";
	FContentTypes[".m4a"]  = "audio/mp4";
	FContentTypes[".mp3"]  = "audio/mpeg";
	FContentTypes[".mp4"]  = "video/mp4";
	FContentTypes[".mpeg"] = "video/mpeg";
	FContentTypes[".mpg"]  = "video/mpeg";
	FContentTypes[".ogg"]  = "application/ogg";
	FContentTypes[".pdf"]  = "application/pdf";
	FContentTypes[".png"]  = "image/png";
	FContentTypes[".rtf"]  = "application/rtf";
	FContentTypes[".svg"]  = "image/svg+xml";
	FContentTypes[".tiff"] = "image/tiff";
	FContentTypes[".txt"]  = "text/plain";
	FContentTypes[".wav"]  = "audio/wav";
	FContentTypes[".webp"] = "image/webp";
	FContentTypes[".xz"]   = "application/x-xz";
	FContentTypes[".zip"]  = "application/zip";

	connect(ui.cmbAccount,SIGNAL(currentIndexChanged(int)),SLOT(onAccountSelected(int)));
	connect(ui.cmbService,SIGNAL(currentIndexChanged(int)),SLOT(onServiceSelected(int)));

	ui.lneFile->setText(AFileName);

	connect(ui.tlbFile,SIGNAL(clicked(bool)),SLOT(onFileButtonClicked(bool)));
	connect(ui.bbxButtons,SIGNAL(clicked(QAbstractButton*)),SLOT(onDialogButtonClicked(QAbstractButton*)));
}

HttpUploadDialog::~HttpUploadDialog()
{
	emit dialogDestroyed();
}

void HttpUploadDialog::onFileButtonClicked(bool)
{
	static QString lastSelectedPath = QDir::homePath();
	QString file = QDir(lastSelectedPath).absoluteFilePath(FFileUpload);
	file = QFileDialog::getOpenFileName(this,tr("Select file to upload"),file);
	if (!file.isEmpty())
	{
		lastSelectedPath = QFileInfo(file).absolutePath();
		FFileUpload = file;
		ui.lneFile->setText(file);
		updateServices();
	}
}

void HttpUploadDialog::onDialogButtonClicked(QAbstractButton *AButton)
{
	if (ui.bbxButtons->standardButton(AButton) == QDialogButtonBox::Ok)
	{
		IHttpUploadService *uploadService;
		ui.cmbAccount->setEnabled(true);
		ui.cmbService->setEnabled(true);
		ui.tlbFile->setEnabled(false);
		ui.lneFile->setReadOnly(true);
		Jid servicJid = ui.cmbService->itemData(ui.cmbService->currentIndex()).toString();
		QList<IHttpUploadService*> services = FAvailableServices;
		for (QList<IHttpUploadService*>::ConstIterator it=services.constBegin(); it!=services.constEnd(); ++it)
		{
			if ((*it)->serviceJid() == servicJid)
			{
				uploadService = (*it);
				break;
			}
		}
		if (uploadService)
		{
			QByteArray contentType = getContentType(ui.lneFile->text()).toUtf8();
			QFile *file = new QFile(ui.lneFile->text());
			file->open(QIODevice::ReadOnly);
			qDebug() << "contentType:" << contentType;
			QFileInfo fileInfo(file->fileName());
			int Id = uploadService->uploadFile(file, contentType, fileInfo.fileName());
			if (Id > 0)
			{
				FFile[uploadService][Id] = file;

				connect(uploadService->instance(),SIGNAL(httpUploadError(int,QString)),SLOT(onHttpUploadError(int,QString)));
				connect(uploadService->instance(),SIGNAL(httpUploadFinished(int,QUrl)),SLOT(onHttpUploadFinished(int,QUrl)));

			}
			else
			{
				file->deleteLater();
			}
		}
	}
}

QString HttpUploadDialog::getContentType(const QString &AFileName)
{
	QFileInfo file(AFileName);
	QString ext = "."+file.suffix();
	QString fExt = "."+file.completeSuffix();

	if (FContentTypes.contains(ext))
	{
		return FContentTypes.value(ext);
	}
	else if (FContentTypes.contains(fExt))
	{
		return FContentTypes.value(fExt);
	}
	else
		return DEFAULT_CONTENT_TYPE;
}

void HttpUploadDialog::onAccountSelected(int AIndex)
{
	ui.cmbService->clear();
	FAvailableServices.clear();
	Jid streamJid = ui.cmbAccount->itemData(AIndex).toString();
	QList<IHttpUploadService*> services = FHttpUpload->availableServices(streamJid);
	for (QList<IHttpUploadService*>::ConstIterator it=services.constBegin(); it!=services.constEnd(); ++it)
	{
		ui.cmbService->addItem((*it)->serviceJid().full(),(*it)->serviceJid().full());
		FAvailableServices.append((*it));
	}

	updateServices();
}

void HttpUploadDialog::onServiceSelected(int AIndex)
{
	int fileSize = QFileInfo(FFileUpload).size();
	QList<IHttpUploadService*> services = FAvailableServices;
	for (QList<IHttpUploadService*>::ConstIterator it=services.constBegin(); it!=services.constEnd(); ++it)
	{
		if ((*it)->serviceJid().full() == ui.cmbService->itemData(AIndex).toString())
		{
			if (fileSize > (*it)->sizeLimit())
			{
				ui.lblSizeLimit->setText(tr("The maximum file size allowed for the selected service is %1!").arg(sizeName((*it)->sizeLimit())));
				//ui.lblSizeLimit->setStyleSheet("color:red");
			}
			else
			{
				ui.lblSizeLimit->setText(tr("%1").arg(sizeName((*it)->sizeLimit())));
			}
		}
	}
}

void HttpUploadDialog::updateServices()
{
	int fileSize = QFileInfo(FFileUpload).size();
	int sizeLimit = 0;

	QList<IHttpUploadService*> services = FAvailableServices;
	for (QList<IHttpUploadService*>::ConstIterator it=services.constBegin(); it!=services.constEnd(); ++it)
	{
		if (!sizeLimit && fileSize <= (*it)->sizeLimit())
		{
			sizeLimit = (*it)->sizeLimit();
			ui.cmbService->setCurrentIndex(ui.cmbService->findData((*it)->serviceJid().full()));
			ui.lblSizeLimit->setText(tr("%1").arg(sizeName(sizeLimit)));
		}
	}
	if (!sizeLimit)
	{
		onServiceSelected(ui.cmbService->currentIndex());
	}
}

// copied from StreamDialog
QString HttpUploadDialog::sizeName(qint64 ABytes) const
{
	static int md[] = {1, 10, 100};
	QString units = tr("B","Byte");
	qreal value = ABytes;

	if (value > 1024)
	{
		value = value / 1024;
		units = tr("KB","Kilobyte");
	}
	if (value > 1024)
	{
		value = value / 1024;
		units = tr("MB","Megabyte");
	}
	if (value > 1024)
	{
		value = value / 1024;
		units = tr("GB","Gigabyte");
	}

	int prec = 0;
	if (value < 10)
		prec = 2;
	else if (value < 100)
		prec = 1;

	while (prec>0 && (qreal)qRound64(value*md[prec-1])/md[prec-1]==(qreal)qRound64(value*md[prec])/md[prec])
		prec--;

	value = (qreal)qRound64(value*md[prec])/md[prec];

	return QString::number(value,'f',prec)+units;
}

void HttpUploadDialog::onHttpUploadError(int AId, const QString &AError)
{
	IHttpUploadService *service = qobject_cast<IHttpUploadService*>(sender());
	if (service)
	{
		QFile *file = FFile[service].take(AId);
		file->deleteLater();

		ui.lblStatus->setText(AError);
		ui.cmbAccount->setEnabled(true);
		ui.cmbService->setEnabled(true);
		ui.tlbFile->setEnabled(true);
		ui.lneFile->setReadOnly(false);
	}
}

void HttpUploadDialog::onHttpUploadFinished(int AId, const QUrl &AUrl)
{
	IHttpUploadService *service = qobject_cast<IHttpUploadService*>(sender());
	if (service)
	{
		QFile *file = FFile[service].take(AId);
		file->deleteLater();

		ui.lblStatus->setText(tr("Complete"));
		ui.cmbAccount->setEnabled(true);
		ui.cmbService->setEnabled(true);
		ui.tlbFile->setEnabled(true);
		ui.lneFile->setReadOnly(false);
	}
}
