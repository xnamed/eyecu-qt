TEMPLATE = subdirs

contains(QT_CONFIG, webkit) {
  SUBDIRS += adiummessagestyle
}

SUBDIRS += optionsmanager
SUBDIRS += xmppstreams
SUBDIRS += iqauth
SUBDIRS += saslauth
SUBDIRS += stanzaprocessor
SUBDIRS += roster
SUBDIRS += presence
SUBDIRS += rostersmodel
SUBDIRS += mainwindow
SUBDIRS += rostersview
SUBDIRS += statuschanger
SUBDIRS += rosterchanger
SUBDIRS += accountmanager
SUBDIRS += traymanager
SUBDIRS += privatestorage
SUBDIRS += messageprocessor
SUBDIRS += messagewidgets
SUBDIRS += messagestyles
SUBDIRS += simplemessagestyle
SUBDIRS += normalmessagehandler
SUBDIRS += chatmessagehandler
SUBDIRS += compress
SUBDIRS += connectionmanager
SUBDIRS += defaultconnection
SUBDIRS += starttls
SUBDIRS += statusicons
SUBDIRS += emoticons
SUBDIRS += clientinfo
SUBDIRS += vcard
SUBDIRS += multiuserchat
SUBDIRS += dataforms
SUBDIRS += servicediscovery
SUBDIRS += bookmarks
SUBDIRS += registration
SUBDIRS += commands
SUBDIRS += jabbersearch
SUBDIRS += gateways
SUBDIRS += messagearchiver
SUBDIRS += privacylists
SUBDIRS += sessionnegotiation
SUBDIRS += avatars
SUBDIRS += notifications
SUBDIRS += autostatus
SUBDIRS += rostersearch
SUBDIRS += console
SUBDIRS += annotations
SUBDIRS += chatstates
SUBDIRS += datastreamsmanager
SUBDIRS += filestreamsmanager
SUBDIRS += filetransfer
SUBDIRS += inbandstreams
SUBDIRS += socksstreams
SUBDIRS += captchaforms
SUBDIRS += bitsofbinary
SUBDIRS += xmppuriqueries
SUBDIRS += remotecontrol
SUBDIRS += pepmanager
SUBDIRS += shortcutmanager 
SUBDIRS += birthdayreminder 
SUBDIRS += urlprocessor
SUBDIRS += filemessagearchive 
SUBDIRS += servermessagearchive 
SUBDIRS += rosteritemexchange 
SUBDIRS += spellchecker 
SUBDIRS += messagecarbons
SUBDIRS += recentcontacts 
SUBDIRS += metacontacts 
SUBDIRS += statistics
SUBDIRS += datastreamspublisher 
