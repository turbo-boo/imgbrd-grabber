#ifndef THROTTLING_MANAGER_H
#define THROTTLING_MANAGER_H

#include <QDateTime>
#include <QMap>
#include <QQueue>


class NetworkReply;

class ThrottlingManager
{
	public:
		ThrottlingManager() = default;

		int interval(int key) const;
		void setInterval(int key, int msInterval);
		void clear();

		int msToRequest(int key, int burst) const;
		void start(int key, NetworkReply *reply, int burst);

	private:
		QMap<int, int> m_intervals;
		QMap<int, QQueue<QDateTime>> m_lastRequests;
};

#endif // THROTTLING_MANAGER_H
