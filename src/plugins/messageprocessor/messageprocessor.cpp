#include "messageprocessor.h"

#include <QDebug>
#include <QVariant>
#include <QTextCursor>
#include <definitions/messagedataroles.h>
#include <definitions/messagewriterorders.h>
#include <definitions/notificationdataroles.h>
#include <definitions/stanzahandlerorders.h>
#include <utils/logger.h>
// *** <<< eyeCU <<< ***
#include <utils/options.h>
#include <definitions/optionvalues.h>
#include <QpXhtml>
// *** >>> eyeCU >>> ***
#define SHC_MESSAGE         "/message"

MessageProcessor::MessageProcessor()
{
	FXmppStreamManager = NULL;
	FStanzaProcessor = NULL;
	FNotifications = NULL;
}

MessageProcessor::~MessageProcessor()
{

}

void MessageProcessor::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Message Manager");
	APluginInfo->description = tr("Allows other modules to send and receive messages");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(XMPPSTREAMS_UUID);
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
}

bool MessageProcessor::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamActiveChanged(IXmppStream *, bool)),SLOT(onXmppStreamActiveChanged(IXmppStream *, bool)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamJidChanged(IXmppStream *, const Jid &)),SLOT(onXmppStreamJidChanged(IXmppStream *, const Jid &)));
		}
	}

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
	{
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("INotifications").value(0,NULL);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
		{
			connect(FNotifications->instance(),SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
			connect(FNotifications->instance(),SIGNAL(notificationRemoved(int)), SLOT(onNotificationRemoved(int)));
		}
	}

	return FStanzaProcessor!=NULL && FXmppStreamManager!=NULL;
}

bool MessageProcessor::initObjects()
{
	insertMessageWriter(MWO_MESSAGEPROCESSOR,this);
	insertMessageWriter(MWO_MESSAGEPROCESSOR_ANCHORS,this);
	return true;
}

bool MessageProcessor::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FActiveStreams.value(AStreamJid) == AHandlerId)
	{
		Message message(AStanza);
		AAccept = sendMessage(AStreamJid,message,IMessageProcessor::DirectionIn) || AAccept;
	}
	return false;
}

bool MessageProcessor::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	if (AOrder == MWO_MESSAGEPROCESSOR)
	{
		if (!AMessage.body(ALang).isEmpty())
			return true;
	}
	return false;
}

bool MessageProcessor::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	bool changed = false;
	if (AOrder == MWO_MESSAGEPROCESSOR)
	{
// *** <<< eyeCU <<< ***
		if (!AMessage.body().isEmpty())
		{
			QTextCursor cursor(ADocument);
			QpXhtml::xmlToText(cursor, convertBodyToHtml(AMessage.body(ALang), Options::node(OPV_MESSAGESTYLE_FONT_MONOSPACED).value().toBool())); // *** <<< eyeCU >>> ***
			changed = true;
		}
// *** >>> eyeCU >>> ***
	}
	else if (AOrder == MWO_MESSAGEPROCESSOR_ANCHORS)
	{
// *** <<< eyeCU <<< ***
//		QRegExp regexp("\\b((https?|ftp)://|www\\.|xmpp:|magnet:|mailto:)\\S+(s|/|#|~|@|&|=|-|\\+|\\*|\\$|\\b|\\\")");
		QRegExp regexp("((https?|ftp)://|(magnet|mailto|xmpp):|(www|ftp)\\.)[^\"\\s$]+[^\"\\.\\s$]");

		regexp.setCaseSensitivity(Qt::CaseInsensitive);
		for (QTextBlock block = ADocument->begin(); block != ADocument->end(); block = block.next())
		{
			QString text;
			int startPos = block.position();
			for (QTextBlock::iterator it=block.begin(); ; it++)
			{
				bool end(false);
				if (it == block.end())
					end = true;
				else
				{
					QTextFragment fragment = it.fragment();
					if (fragment.charFormat().isAnchor() &&
						fragment.charFormat().hasProperty(QTextFormat::AnchorHref))
						end = true;
					else
						text.append(fragment.text());
				}

				if (end)
				{
					if (!text.isEmpty())
					{
						for (int i = text.indexOf(regexp); i !=- 1;
							 i = text.indexOf(regexp, i))
						{
							QTextCursor cursor(ADocument);
							cursor.setPosition(startPos+i);
							cursor.movePosition(QTextCursor::Right,
												QTextCursor::KeepAnchor,
												regexp.matchedLength());

							QTextCharFormat linkFormat;
							linkFormat.setAnchor(true);
							linkFormat.setAnchorHref(QUrl::fromUserInput(cursor.selectedText()).toString());
							cursor.mergeCharFormat(linkFormat);
							changed = true;
							i += regexp.matchedLength();
						}
					}

					if (it == block.end())
						break;
				}
			}
		}
// *** >>> eyeCU >>> ***
	}
	return changed;
}

bool MessageProcessor::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
{
	bool changed = false;
	if (AOrder == MWO_MESSAGEPROCESSOR)
	{
		QString body = convertTextToBody(ADocument->toPlainText());
		if (!body.isEmpty())
		{
			AMessage.setBody(body,ALang);
			changed = true;
		}
	}
	return changed;
}

QList<Jid> MessageProcessor::activeStreams() const
{
	return FActiveStreams.keys();
}

bool MessageProcessor::isActiveStream(const Jid &AStreamJid) const
{
	return FActiveStreams.contains(AStreamJid);
}

void MessageProcessor::appendActiveStream(const Jid &AStreamJid)
{
	if (FStanzaProcessor && AStreamJid.isValid() && !FActiveStreams.contains(AStreamJid))
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.order = SHO_DEFAULT;
		shandle.direction = IStanzaHandle::DirectionIn;
		shandle.streamJid = AStreamJid;
		shandle.conditions.append(SHC_MESSAGE);
		FActiveStreams.insert(shandle.streamJid,FStanzaProcessor->insertStanzaHandle(shandle));
		emit activeStreamAppended(AStreamJid);
	}
}

void MessageProcessor::removeActiveStream(const Jid &AStreamJid)
{
	if (FStanzaProcessor && FActiveStreams.contains(AStreamJid))
	{
		FStanzaProcessor->removeStanzaHandle(FActiveStreams.take(AStreamJid));
		foreach(int notifyId, FNotifyId2MessageId.keys())
		{
			INotification notify = FNotifications->notificationById(notifyId);
			if (AStreamJid == notify.data.value(NDR_STREAM_JID).toString())
				removeMessageNotify(FNotifyId2MessageId.value(notifyId));
		}
		emit activeStreamRemoved(AStreamJid);
	}
}

bool MessageProcessor::sendMessage(const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	if (processMessage(AStreamJid,AMessage,ADirection))
	{
		if (ADirection == IMessageProcessor::DirectionOut)
		{
			Stanza stanza = AMessage.stanza(); // Ignore changes in StanzaProcessor
			if (FStanzaProcessor && FStanzaProcessor->sendStanzaOut(AStreamJid,stanza))
			{
				displayMessage(AStreamJid,AMessage,ADirection);
				emit messageSent(AMessage);
				return true;
			}
		}
		else 
		{
			displayMessage(AStreamJid,AMessage,ADirection);
			emit messageReceived(AMessage);
			return true;
		}
	}
	return false;
}

bool MessageProcessor::processMessage(const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	if (ADirection == IMessageProcessor::DirectionIn)
		AMessage.setTo(AStreamJid.full());
	else
		AMessage.setFrom(AStreamJid.full());

	if (AMessage.data(MDR_MESSAGE_ID).isNull())
		AMessage.setData(MDR_MESSAGE_ID,newMessageId());
	AMessage.setData(MDR_MESSAGE_DIRECTION,ADirection);

	bool hooked = false;
	QMapIterator<int,IMessageEditor *> it(FMessageEditors);
	ADirection == DirectionIn ? it.toFront() : it.toBack();
	while (!hooked && (ADirection==DirectionIn ? it.hasNext() : it.hasPrevious()))
	{
		ADirection == DirectionIn ? it.next() : it.previous();
		hooked = it.value()->messageReadWrite(it.key(), AStreamJid, AMessage, ADirection);
	}

	return !hooked;
}

bool MessageProcessor::displayMessage(const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AStreamJid);
	IMessageHandler *handler = findMessageHandler(AMessage,ADirection);
	if (handler && handler->messageDisplay(AMessage,ADirection))
	{
		notifyMessage(handler,AMessage,ADirection);
		return true;
	}
	return false;
}

QList<int> MessageProcessor::notifiedMessages() const
{
	return FNotifiedMessages.keys();
}

Message MessageProcessor::notifiedMessage(int AMesssageId) const
{
	return FNotifiedMessages.value(AMesssageId);
}

int MessageProcessor::notifyByMessage(int AMessageId) const
{
	return FNotifyId2MessageId.key(AMessageId,-1);
}

int MessageProcessor::messageByNotify(int ANotifyId) const
{
	return FNotifyId2MessageId.value(ANotifyId,-1);
}

void MessageProcessor::showNotifiedMessage(int AMessageId)
{
	IMessageHandler *handler = FHandlerForMessage.value(AMessageId);
	if (handler)
		handler->messageShowNotified(AMessageId);
}

void MessageProcessor::removeMessageNotify(int AMessageId)
{
	int notifyId = FNotifyId2MessageId.key(AMessageId);
	if (notifyId > 0)
	{
		FNotifiedMessages.remove(AMessageId);
		FNotifyId2MessageId.remove(notifyId);
		FHandlerForMessage.remove(AMessageId);
		FNotifications->removeNotification(notifyId);
		emit messageNotifyRemoved(AMessageId);
	}
}


bool MessageProcessor::messageHasText(const Message &AMessage, const QString &ALang) const
{
	Message messageCopy = AMessage;
	QMapIterator<int,IMessageWriter *> it(FMessageWriters); it.toFront();
	while (it.hasNext())
	{
		if (it.next().value()->writeMessageHasText(it.key(),messageCopy,ALang))
			return true;
	}
	return false;
}

bool MessageProcessor::messageToText(const Message &AMessage, QTextDocument *ADocument, const QString &ALang) const
{
	bool changed = false;

	Message messageCopy = AMessage;
	QMapIterator<int,IMessageWriter *> it(FMessageWriters); it.toFront();
	while (it.hasNext())
		changed = it.next().value()->writeMessageToText(it.key(),messageCopy,ADocument,ALang) || changed;

	return changed;
}

bool MessageProcessor::textToMessage(const QTextDocument *ADocument, Message &AMessage, const QString &ALang) const
{
	bool changed = false;
	
	QTextDocument *documentCopy = ADocument->clone();
	QMapIterator<int,IMessageWriter *> it(FMessageWriters); it.toBack();
	while (it.hasPrevious())
		changed = it.previous().value()->writeTextToMessage(it.key(),documentCopy,AMessage,ALang) || changed;
	delete documentCopy;

	return changed;
}

IMessageWindow *MessageProcessor::getMessageWindow(const Jid &AStreamJid, const Jid &AContactJid, Message::MessageType AType, int AAction) const
{
	for (QMultiMap<int, IMessageHandler *>::const_iterator it = FMessageHandlers.constBegin(); it!=FMessageHandlers.constEnd(); ++it)
	{
		IMessageWindow *window = it.value()->messageGetWindow(AStreamJid,AContactJid,AType);
		if (window)
		{
			if (AAction == ActionAssign)
				window->assignTabPage();
			else if (AAction == ActionShowNormal)
				window->showTabPage();
			else if (AAction == ActionShowMinimized)
				window->showMinimizedTabPage();
			return window;
		}
	}
	return NULL;
}

QMultiMap<int, IMessageHandler *> MessageProcessor::messageHandlers() const
{
	return FMessageHandlers;
}

void MessageProcessor::insertMessageHandler(int AOrder, IMessageHandler *AHandler)
{
	if (AHandler && !FMessageHandlers.contains(AOrder,AHandler))
		FMessageHandlers.insertMulti(AOrder,AHandler);
}

void MessageProcessor::removeMessageHandler(int AOrder, IMessageHandler *AHandler)
{
	if (FMessageHandlers.contains(AOrder,AHandler))
		FMessageHandlers.remove(AOrder,AHandler);
}

QMultiMap<int, IMessageWriter *> MessageProcessor::messageWriters() const
{
	return FMessageWriters;
}

void MessageProcessor::insertMessageWriter(int AOrder, IMessageWriter *AWriter)
{
	if (AWriter && !FMessageWriters.contains(AOrder,AWriter))
		FMessageWriters.insertMulti(AOrder,AWriter);
}

void MessageProcessor::removeMessageWriter(int AOrder, IMessageWriter *AWriter)
{
	if (FMessageWriters.contains(AOrder,AWriter))
		FMessageWriters.remove(AOrder,AWriter);
}

QMultiMap<int, IMessageEditor *> MessageProcessor::messageEditors() const
{
	return FMessageEditors;
}

void MessageProcessor::insertMessageEditor(int AOrder, IMessageEditor *AEditor)
{
	if (AEditor && !FMessageEditors.contains(AOrder,AEditor))
		FMessageEditors.insertMulti(AOrder,AEditor);
}

void MessageProcessor::removeMessageEditor(int AOrder, IMessageEditor *AEditor)
{
	if (FMessageEditors.contains(AOrder,AEditor))
		FMessageEditors.remove(AOrder,AEditor);
}

int MessageProcessor::newMessageId()
{
	static int messageId = 1;
	return messageId++;
}

IMessageHandler *MessageProcessor::findMessageHandler(const Message &AMessage, int ADirection)
{
	for (QMultiMap<int, IMessageHandler *>::const_iterator it = FMessageHandlers.constBegin(); it!=FMessageHandlers.constEnd(); ++it)
		if (it.value()->messageCheck(it.key(),AMessage,ADirection))
			return it.value();
	return NULL;
}

void MessageProcessor::notifyMessage(IMessageHandler *AHandler, const Message &AMessage, int ADirection)
{
	if (FNotifications && AHandler)
	{
		INotification notify = AHandler->messageNotify(FNotifications, AMessage, ADirection);
		if (notify.kinds > 0)
		{
			int notifyId = FNotifications->appendNotification(notify);
			int messageId = AMessage.data(MDR_MESSAGE_ID).toInt();
			FNotifiedMessages.insert(messageId,AMessage);
			FNotifyId2MessageId.insert(notifyId,messageId);
			FHandlerForMessage.insert(messageId,AHandler);
			emit messageNotifyInserted(messageId);
		}
	}
}

QString MessageProcessor::convertTextToBody(const QString &AString) const
{
	QString result = AString;
	result.remove(QChar::Null);
	result.remove(QChar::ObjectReplacementCharacter);
	return result;
}

QDomDocument MessageProcessor::convertBodyToHtml(const QString &AString, bool AMonospaced) const
{
	// *** <<< eyeCU <<< ***
    QDomDocument doc;	
	QDomElement code=doc.createElement(AMonospaced?"code":"span");
    code.setAttribute("style", "white-space: pre-wrap;");
    doc.appendChild(code);
    QStringList splitted=AString.split('\n');
    for (QStringList::ConstIterator it=splitted.constBegin(); it!=splitted.constEnd(); it++)
    {
        if (it!=splitted.constBegin())
           code.appendChild(doc.createElement("br"));
        code.appendChild(doc.createTextNode(*it));
    }
    return doc;
	// *** >>> eyeCU >>> ***
}

void MessageProcessor::onNotificationActivated(int ANotifyId)
{
	if (FNotifyId2MessageId.contains(ANotifyId))
		showNotifiedMessage(FNotifyId2MessageId.value(ANotifyId));
}

void MessageProcessor::onNotificationRemoved(int ANotifyId)
{
	if (FNotifyId2MessageId.contains(ANotifyId))
		removeMessageNotify(FNotifyId2MessageId.value(ANotifyId));
}

void MessageProcessor::onXmppStreamActiveChanged(IXmppStream *AXmppStream, bool AActive)
{
	if (AActive)
		appendActiveStream(AXmppStream->streamJid());
	else
		removeActiveStream(AXmppStream->streamJid());
}

void MessageProcessor::onXmppStreamJidChanged(IXmppStream *AXmppStream, const Jid &ABefore)
{
	if (FActiveStreams.contains(ABefore))
	{
		int handleId = FActiveStreams.take(ABefore);
		FActiveStreams.insert(AXmppStream->streamJid(),handleId);
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_messageprocessor, MessageProcessor)
#endif
