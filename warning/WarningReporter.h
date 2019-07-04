/*
  ==============================================================================

    WarningReporter.h
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class WarningReporter
{
public:
	juce_DeclareSingleton(WarningReporter, true);

	Array<WarningTarget*> targets;

	WarningReporter();
	~WarningReporter();

	void registerWarning(WarningTarget*);
	void unregisterWarning(WarningTarget*);

	// ASYNC
	class  WarningReporterEvent
	{
	public:
		enum Type { WARNING_REGISTERED, WARNING_UNREGISTERED };

		WarningReporterEvent(Type t, WarningTarget * target) :
			type(t), target(target) {}

		Type type;
		WarningTarget* target;
	};

	QueuedNotifier<WarningReporterEvent> warningReporterNotifier;
	typedef QueuedNotifier<WarningReporterEvent>::Listener AsyncListener;

	void addAsyncWarningReporterListener(AsyncListener* newListener) { warningReporterNotifier.addListener(newListener); }
	void addAsyncCoalescedWarningReporterListener(AsyncListener* newListener) { warningReporterNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncWarningReporterListener(AsyncListener* listener) { warningReporterNotifier.removeListener(listener); }

};
