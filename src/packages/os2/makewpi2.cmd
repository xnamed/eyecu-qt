@echo on
set packagename=eyecu2
set devpackagename=%packagename%-dev
set version=1.3.0.20160425
set packagefilename=%packagename%-%version%
set devpackagefilename=%devpackagename%-%version%

echo Creating base package
call substver.cmd %packagename% %version%
del %packagefilename%.wpi

echo packagefilename=%packagefilename%

wic -a %packagefilename%.wpi 1 -r -cc:\eyecu eyecu.exe eyecutls.dll plugins\accntmgr.dll plugins\chatmsgh.dll plugins\connectm.dll plugins\dfltconn.dll plugins\mainwndw.dll plugins\msgprcsr.dll plugins\msgstyle.dll plugins\msgwdgts.dll plugins\nrmlmsgh.dll plugins\notifics.dll plugins\optsmngr.dll plugins\presence.dll plugins\roster.dll plugins\rstrchng.dll plugins\rstrmodl.dll plugins\rstrview.dll plugins\saslauth.dll plugins\smpmsgst.dll plugins\stanzapr.dll plugins\starttls.dll plugins\statusch.dll plugins\sttsicns.dll plugins\traymngr.dll plugins\xmppstrm.dll resources\menuicons\* resources\statusicons\shared\* resources\simplemessagestyles\* resources\sounds\shared\* 2 shadow
wic -a %packagefilename%.wpi 4 -cc:\eyecu plugins\statstcs.dll 5 -cc:\eyecu plugins\privlsts.dll 6 -cc:\eyecu plugins\compress.dll 7 -r -cc:\eyecu plugins\vcard.dll 8 -r -cc:\eyecu plugins\birthday.dll 9 -cc:\eyecu plugins\avatars.dll
wic -a %packagefilename%.wpi 10 -cc:\eyecu plugins\adiummsg.dll resources\adiummessagestyles\shared\* 11 -r -cc:\eyecu resources\adiummessagestyles\renkoo\* 12 -r -cc:\eyecu resources\adiummessagestyles\yMous\* 13 -cc:\eyecu plugins\autostts.dll 14 -cc:\eyecu plugins\iqauth.dll
wic -a %packagefilename%.wpi 15 -cc:\eyecu plugins\gateways.dll 16 -cc:\eyecu plugins\shcutmgr.dll 17 -cc:\eyecu plugins\srvdisco.dll resources\serviceicons\shared\* 18 -cc:\eyecu plugins\msgcrbns.dll 19 -cc:\eyecu plugins\console.dll
wic -a %packagefilename%.wpi 20 -cc:\eyecu plugins\msgarchv.dll 21 -cc:\eyecu plugins\fmsgarch.dll 22 -cc:\eyecu plugins\smsgarch.dll 25 -cc:\eyecu plugins\rstrsrch.dll 26 -cc:\eyecu plugins\jbbrsrch.dll 27 -cc:\eyecu plugins\ritxchng.dll 28 -cc:\eyecu plugins\muchat.dll 29 -cc:\eyecu plugins\chatstat.dll
wic -a %packagefilename%.wpi 30 -cc:\eyecu plugins\privstor.dll 31 -cc:\eyecu plugins\metacnts.dll 32 -cc:\eyecu plugins\bookmark.dll 33 -cc:\eyecu plugins\annotats.dll 34 -cc:\eyecu plugins\rccntcts.dll 35 -cc:\eyecu plugins\clntinfo.dll 36 -cc:\eyecu plugins\urlproc.dll 37 -cc:\eyecu plugins\bitsobin.dll 38 -cc:\eyecu plugins\boburlhd.dll 39 -cc:\eyecu plugins\xmppuriq.dll
wic -a %packagefilename%.wpi 40 -cc:\eyecu plugins\spellchk.dll 41 -cc:\eyecu hunspell\en_GB.* 42 -cc:\eyecu hunspell\en_US.* 43 -cc:\eyecu hunspell\en_NZ.* 44 -cc:\eyecu hunspell\en_CA.* 45 -cc:\eyecu hunspell\en_AU.* 46 -cc:\eyecu hunspell\en_ZA.* 47 -cc:\eyecu hunspell\ru_RU.* 48 -cc:\eyecu hunspell\russian-aot-ieyo.* 49 -cc:\eyecu hunspell\russian-rk-ieyo.* 50 -cc:\eyecu hunspell\de_AT_frami.* 51 -cc:\eyecu hunspell\de_DE_frami.* 52 -cc:\eyecu hunspell\de_DE_igerman98.* 53 -cc:\eyecu hunspell\pl_PL.* 54 -cc:\eyecu hunspell\uk_UA.* 55 -cc:\eyecu hunspell\nl_NL.* 56 -cc:\eyecu hunspell\es_ES.*
wic -a %packagefilename%.wpi 60 -cc:\eyecu\resources\statusicons\aim * 61 -cc:\eyecu\resources\statusicons\bot * 62 -cc:\eyecu\resources\statusicons\car * 63 -cc:\eyecu\resources\statusicons\conference * 64 -cc:\eyecu\resources\statusicons\facebook * 65 -cc:\eyecu\resources\statusicons\gadu * 66 -cc:\eyecu\resources\statusicons\gtalk * 67 -cc:\eyecu\resources\statusicons\icq * 68 -cc:\eyecu\resources\statusicons\livejournal * 69 -cc:\eyecu\resources\statusicons\mrim * 70 -cc:\eyecu\resources\statusicons\msn * 71 -cc:\eyecu\resources\statusicons\odnoklassniki * 72 -cc:\eyecu\resources\statusicons\rss * 73 -cc:\eyecu\resources\statusicons\skype * 74 -cc:\eyecu\resources\statusicons\sms * 75 -cc:\eyecu\resources\statusicons\smtp * 76 -cc:\eyecu\resources\statusicons\twitter * 77 -cc:\eyecu\resources\statusicons\vkontakte * 78 -cc:\eyecu\resources\statusicons\weather * 79 -cc:\eyecu\resources\statusicons\yahoo * 80 -cc:\eyecu\resources\statusicons\yaonline *
wic -a %packagefilename%.wpi 85 -cc:\eyecu plugins\xhtmlim.dll resources\xhtml\shared\* 
wic -a %packagefilename%.wpi 90 -cc:\eyecu plugins\emoticon.dll 91 -cc:\eyecu\resources\emoticons\default * 92 -cc:\eyecu\resources\emoticons\blobs_purple *
wic -a %packagefilename%.wpi 93 -cc:\eyecu plugins\emoji.dll resources\emoji\emoji.json resources\emoji\category_icons\* 94 -c"c:\eyecu\resources\emoji\assets\Emoji One\png\16" * 95 -c"c:\eyecu\resources\emoji\assets\Emoji One\png\32" * 97 -c"c:\eyecu\resources\emoji\assets\Emoji One\png\48" *
wic -a %packagefilename%.wpi 103 -cc:\eyecu plugins\map.dll resources\mapicons\shared\* 104 -cc:\eyecu plugins\msosm.dll 105 -cc:\eyecu plugins\mswiki.dll 106 -cc:\eyecu plugins\msgoogle.dll 107 -cc:\eyecu plugins\msyahoo.dll 108 -cc:\eyecu plugins\msbing.dll 109 -cc:\eyecu plugins\msovi.dll 110 -cc:\eyecu plugins\msrumap.dll 111 -cc:\eyecu plugins\msvitel.dll 112 -cc:\eyecu plugins\msesri.dll
wic -a %packagefilename%.wpi 113 -cc:\eyecu plugins\msnavteq.dll 114 -cc:\eyecu plugins\msyandex.dll 115 -cc:\eyecu plugins\msnavitl.dll 116 -cc:\eyecu plugins\ms2gis.dll 117 -cc:\eyecu plugins\mskosmos.dll 119 -cc:\eyecu plugins\msrosrsr.dll 120 -cc:\eyecu plugins\msmegafn.dll 121 -cc:\eyecu plugins\msprogrd.dll
wic -a %packagefilename%.wpi 125 -cc:\eyecu plugins\mpcntcts.dll 126 -cc:\eyecu plugins\mapmessg.dll 127 -cc:\eyecu plugins\conproxn.dll 130 -cc:\eyecu plugins\mpsearch.dll 131 -cc:\eyecu plugins\msposm.dll 132 -cc:\eyecu plugins\mspgoogl.dll 133 -cc:\eyecu plugins\msp2gis.dll 134 -cc:\eyecu plugins\mspyandx.dll 135 -cc:\eyecu plugins\mspnavtl.dll resources\navitel\shared\* 136 -cc:\eyecu plugins\msphere.dll 137 -cc:\eyecu plugins\mpmagnif.dll 138 -cc:\eyecu plugins\maplcsel.dll
wic -a %packagefilename%.wpi 140 -cc:\eyecu plugins\datafrms.dll 141 -cc:\eyecu plugins\commands.dll 142 -cc:\eyecu plugins\captchaf.dll 143 -cc:\eyecu plugins\remtctrl.dll
wic -a %packagefilename%.wpi 145 -cc:\eyecu plugins\plceview.dll 146 -cc:\eyecu plugins\pvpgoogl.dll 147 -cc:\eyecu plugins\strtview.dll 148 -cc:\eyecu plugins\svpgoogl.dll
wic -a %packagefilename%.wpi 150 -r -cc:\eyecu resources\typepoint\* resources\country\shared\* plugins\poi.dll 152 -cc:\eyecu plugins\nickname.dll 153 -cc:\eyecu plugins\abbrvtns.dll resources\abbreviations\shared\* 156 -cc:\eyecu plugins\receipts.dll 157 -cc:\eyecu plugins\attntion.dll 158 -cc:\eyecu plugins\oob.dll 159 -cc:\eyecu plugins\clnticns.dll resources\clienticons\shared\*
wic -a %packagefilename%.wpi 160 -cc:\eyecu plugins\pepmangr.dll 161 -cc:\eyecu plugins\mood.dll resources\moodicons\shared\* 162 -cc:\eyecu plugins\activity.dll resources\activityicons\shared\* 163 -cc:\eyecu plugins\geoloc.dll
wic -a %packagefilename%.wpi 164 -cc:\eyecu plugins\tune.dll 165 -cc:\eyecu plugins\tlfile.dll 166 -cc:\eyecu plugins\tlpm123.dll 167 -cc:\eyecu plugins\tlquplay.dll 168 -cc:\eyecu plugins\tlz.dll 169 -cc:\eyecu plugins\tirlstfm.dll
wic -a %packagefilename%.wpi 170 -cc:\eyecu plugins\position.dll 171 -cc:\eyecu plugins\pmmanual.dll 172 -cc:\eyecu plugins\pmserial.dll 173 -cc:\eyecu plugins\pmip.dll 174 -cc:\eyecu plugins\pmipfree.dll
wic -a %packagefilename%.wpi 180 -cc:\eyecu resources\wizards\shared\wizard.def.xml resources\wizards\shared\wizard.png resources\wizards\shared\banner.png
wic -a %packagefilename%.wpi 181 -cc:\eyecu plugins\waccount.dll resources\wizards\shared\*.html resources\wizards\shared\servers.xml resources\wizards\shared\software.def.xml resources\wizards\shared\networks.def.xml resources\wizards\shared\ejabberd.png resources\wizards\shared\jabberd.png resources\wizards\shared\openfire.png resources\wizards\shared\prosody.png resources\wizards\shared\tigase.png resources\wizards\shared\gtalk.png resources\wizards\shared\livejournal.png resources\wizards\shared\odnoklassniki.png resources\wizards\shared\qip.png resources\wizards\shared\xmpp.png resources\wizards\shared\yaonline.png resources\wizards\shared\account.png resources\wizards\shared\accountend.png
wic -a %packagefilename%.wpi 182 -cc:\eyecu plugins\wtrnsprt.dll resources\wizards\shared\transports.xml resources\wizards\shared\transport.png resources\wizards\shared\transportend.png resources\wizards\shared\yes.png resources\wizards\shared\no.png
wic -a %packagefilename%.wpi 183 -cc:\eyecu plugins\registrn.dll 184 -cc:\eyecu plugins\sessnego.dll 185 -cc:\eyecu plugins\filetran.dll plugins\fstrmmgr.dll plugins\dstrmmgr.dll 186 -cc:\eyecu plugins\sockstrm.dll 187 -cc:\eyecu plugins\ibbstrms.dll 188 -cc:\eyecu plugins\dstrmpbl.dll
copy README.OS2 c:\eyecu
wic -a %packagefilename%.wpi 189 -cc:\eyecu AUTHORS CHANGELOG COPYING README TRANSLATORS README.OS2 
del c:\eyecu\README.OS2
wic -a %packagefilename%.wpi 190 -cc:\eyecu\translations en\*.qm
wic -a %packagefilename%.wpi 191 -cc:\eyecu\translations ru\*.qm
wic -a %packagefilename%.wpi 192 -cc:\eyecu\translations uk\*.qm
wic -a %packagefilename%.wpi 193 -cc:\eyecu\translations pl\*.qm
wic -a %packagefilename%.wpi 194 -cc:\eyecu\translations de\*.qm
wic -a %packagefilename%.wpi 195 -cc:\eyecu\translations es\*.qm
wic -a %packagefilename%.wpi 196 -cc:\eyecu\translations nl\*.qm
wic -a %packagefilename%.wpi 197 -cc:\eyecu\translations ja\*.qm
wic -a %packagefilename%.wpi 198 -r -cc:\eyecu *.sym
wic -a %packagefilename%.wpi -s %packagename%.wis 

echo Creating Dev package
call substver.cmd %devpackagename% %version%
del %devpackagefilename%.wpi
wic -a %devpackagefilename%.wpi 199 -cc:\eyecu eyecutls.lib
wic -a %devpackagefilename%.wpi 199 -cc:\eyecu\sdk definitions\* utils\* * interfaces\iaccountmanager.h interfaces\iannotations.h interfaces\iautostatus.h interfaces\iavatars.h interfaces\ibirthdayreminder.h interfaces\ibitsofbinary.h interfaces\ibookmarks.h interfaces\icaptchaforms.h interfaces\ichatstates.h interfaces\iclientinfo.h interfaces\icommands.h interfaces\iconnectionmanager.h interfaces\idataforms.h interfaces\idatastreamsmanager.h interfaces\idefaultconnection.h interfaces\iemoticons.h interfaces\ifilemessagearchive.h interfaces\ifilestreamsmanager.h interfaces\ifiletransfer.h interfaces\igateways.h interfaces\iinbandstreams.h interfaces\ijabbersearch.h interfaces\imainwindow.h interfaces\imessagearchiver.h interfaces\imessagecarbons.h
wic -a %devpackagefilename%.wpi 199 -cc:\eyecu\sdk interfaces\imessageprocessor.h interfaces\imessagestyles.h interfaces\imessagewidgets.h interfaces\imetacontacts.h interfaces\imultiuserchat.h interfaces\inotifications.h interfaces\ioptionsmanager.h interfaces\ipepmanager.h interfaces\ipluginmanager.h interfaces\ipositioning.h interfaces\ipresence.h interfaces\iprivacylists.h interfaces\iprivatestorage.h interfaces\ireceipts.h interfaces\irecentcontacts.h interfaces\iregistraton.h interfaces\iroster.h interfaces\irosterchanger.h interfaces\irosteritemexchange.h interfaces\irostersearch.h interfaces\irostersmodel.h interfaces\irostersview.h interfaces\iservermessagearchive.h interfaces\iservicediscovery.h interfaces\isessionnegotiation.h interfaces\isocksstreams.h
wic -a %devpackagefilename%.wpi 199 -cc:\eyecu\sdk interfaces\ispellchecker.h interfaces\istanzaprocessor.h interfaces\istatistics.h interfaces\istatuschanger.h interfaces\istatusicons.h interfaces\itraymanager.h interfaces\iurlprocessor.h interfaces\ivcard.h interfaces\ixmppstreams.h interfaces\ixmppuriqueries.h ixhtmlim.h
wic -a %devpackagefilename%.wpi 199 -r -cc:\eyecu\sdk interfaces\iabbreviations.h interfaces\iactivity.h interfaces\iattention.h interfaces\iclienticons.h interfaces\igeoloc.h interfaces\imap.h interfaces\imapcontacts.h interfaces\imaplocationselector.h interfaces\imapmagnifier.h interfaces\imapmessage.h interfaces\imapscene.h interfaces\imapsearch.h interfaces\imood.h interfaces\imultiuserchat.h interfaces\inickname.h interfaces\ioob.h interfaces\ipoi.h interfaces\ipositioning.h interfaces\ireceipts.h interfaces\istreetview.h interfaces\itune.h interfaces\iwizardaccount.h interfaces\iwizardtransport.h
wic -a %devpackagefilename%.wpi -s %devpackagename%.wis 