#include "throttling-manager.h"
#include <QTimer>
#include "network-reply.h"


int ThrottlingManager::interval(int key) const
{
	return m_intervals[key];
}

void ThrottlingManager::setInterval(int key, int msInterval)
{
	m_intervals[key] = msInterval;
}

void ThrottlingManager::clear()
{
	m_intervals.clear();
	m_lastRequests.clear();
}


int ThrottlingManager::msToRequest(int key, int burst) const
{
	const int msInterval = interval(key);
	if (msInterval <= 0 || burst <= 0) {
		return 0;
	}

	const auto it = m_lastRequests.constFind(key);
	if (it == m_lastRequests.constEnd() || it->isEmpty()) {
		return 0;
	}

	const QDateTime now = QDateTime::currentDateTime();
	const QQueue<QDateTime> &queue = it.value();
	int recentCount = 0;
	QDateTime oldestRecent;
	for (const QDateTime &dt : queue) {
		if (dt.msecsTo(now) < msInterval) {
			recentCount++;
			if (!oldestRecent.isValid() || dt < oldestRecent) {
				oldestRecent = dt;
			}
		}
	}
	if (recentCount < burst || !oldestRecent.isValid()) {
		return 0;
	}

	const qint64 sinceOldest = oldestRecent.msecsTo(now);
	return qMax(0, msInterval - static_cast<int>(sinceOldest));
}

void ThrottlingManager::start(int key, NetworkReply *reply, int burst)
{
	const int msInterval = interval(key);
	if (msInterval <= 0 || burst <= 0) {
		reply->start(0);
		return;
	}

	QDateTime now = QDateTime::currentDateTime();
	QQueue<QDateTime> &queue = m_lastRequests[key];

	// Drop requests older than the interval window.
	while (!queue.isEmpty() && queue.head().msecsTo(now) >= msInterval) {
		queue.dequeue();
	}

	int msWait = 0;
	if (queue.size() >= burst) {
		const QDateTime oldest = queue.head();
		msWait = qMax(0, msInterval - static_cast<int>(oldest.msecsTo(now)));
	}

	const QDateTime scheduled = now.addMSecs(msWait);
	queue.enqueue(scheduled);
	reply->start(msWait);
}
