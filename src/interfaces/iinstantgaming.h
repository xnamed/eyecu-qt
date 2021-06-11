#ifndef IINSTANTGAMING_H
#define IINSTANTGAMING_H

#include <QObject>

#define INSTANTGAMING_UUID "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"

class IInstantGaming {
public:
	virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IInstantGaming, "RWS.Plugin.IInstantGaming/1.0")

#endif // IINSTANTGAMING_H
