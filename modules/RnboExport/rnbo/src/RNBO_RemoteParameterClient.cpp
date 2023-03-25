//
//  RNBO_RemoteParameterClient.cpp
//  RNBOApp
//
//  Created by Rob Sussman on 11/20/15.
//
//

#include "RNBO.h"

#ifdef USE_REMOTE_PARAMETER_CLIENT

#include "RNBO_Engine.h"
#include <juce_events/juce_events.h>
#include <map>

namespace RNBO {

	class ClientConnectionHandler;

	class RemoteParameterServerImpl : public juce::InterprocessConnectionServer
	{
	public:

		RemoteParameterServerImpl(Engine& engine)
		: _engine(engine)
		{
		}

		virtual ~RemoteParameterServerImpl()
		{

		}

		void removeConnectionObject(juce::InterprocessConnection* connection);

	protected:
		juce::InterprocessConnection* createConnectionObject() override;

	private:
		class Engine& _engine;
		std::vector<std::unique_ptr<ClientConnectionHandler>> _activeConnections;
	};

	RemoteParameterServer::RemoteParameterServer(int port)
	: _port(port)
	, _deleteJuceMessingServer(false)
	{
		if (!juce::MessageManager::getInstanceWithoutCreating()) {
			_deleteJuceMessingServer = true;
			juce::MessageManager::getInstance();
		}
	}

	RemoteParameterServer::~RemoteParameterServer()
	{
		stop();
		if (_deleteJuceMessingServer) {
			juce::MessageManager::deleteInstance();
		}
	}

	bool RemoteParameterServer::start(Engine& engine)
	{
		_server = make_unique<RemoteParameterServerImpl>(engine);
		if (!_server) {
			return false;
		}
		bool success = _server->beginWaitingForSocket(_port);
		return success;
	}

	void RemoteParameterServer::stop()
	{
		if (_server) {
			_server->stop();
			_server = nullptr;
		}
	}

	class ClientConnectionHandler
	: public juce::InterprocessConnection
	, public juce::AsyncUpdater
	, public EventHandler
	{
	public:
		enum {
			RNBO_VERB_UNKNOWN = 0,
			RNBO_PARAM = 1,
			RNBO_PROBE_RMS = 2,
			RNBO_PROBE_PEAK = 3,
			RNBO_PROBE_ASSIGN = 4,
			RNBO_PROBE_END = 5,
			RNBO_LISTEN_ON = 6,
			RNBO_LISTEN_OFF = 7,
			RNBO_FREEZE_ON = 8,
			RNBO_FREEZE_OFF = 9,
			RNBO_FREEZE_VECTOR = 10
		};

		ClientConnectionHandler(Engine& engine, RemoteParameterServerImpl* server)
		: _engine(engine)
		, _server(server)
		{
			// Log() << "ClientConnectionHandler::ClientConnectionHandler: this=" << this;
			_verbs.insert(std::pair<String, int>("param", RNBO_PARAM));
			_verbs.insert(std::pair<String, int>("probe_assign", RNBO_PROBE_ASSIGN));
			_verbs.insert(std::pair<String, int>("probe_rms", RNBO_PROBE_RMS));
			_verbs.insert(std::pair<String, int>("probe_peak", RNBO_PROBE_PEAK));
			_verbs.insert(std::pair<String, int>("probe_end", RNBO_PROBE_END));
			_verbs.insert(std::pair<String, int>("listen_on", RNBO_LISTEN_ON));
			_verbs.insert(std::pair<String, int>("listen_off", RNBO_LISTEN_OFF));
			_verbs.insert(std::pair<String, int>("freeze_on", RNBO_FREEZE_ON));
			_verbs.insert(std::pair<String, int>("freeze_off", RNBO_FREEZE_OFF));
			_verbs.insert(std::pair<String, int>("freeze_vector", RNBO_FREEZE_VECTOR));

			// Let's try using MultiProducer and we could allow JUCE to deliver
			// messages from any thread instead of moving all networking to main thread
			// thus avoiding serializing with UI.
			// Alternatively, could use SingleProducer and have JUCE move networking to main thread
			// but I suspect that could be worse.
			_probeID = "";
			_probeParameterIndexPeak = -1;
			_probeParameterIndexRMS = -1;
			_probingSignalIndex = -1;
			_parameterInterface = _engine.createParameterInterface(ParameterEventInterface::MultiProducer, this);
		}

		~ClientConnectionHandler()
		{
			// Log() << "ClientConnectionHandler::~ClientConnectionHandler: this=" << this;
		}

		// juce::InterprocessConnection

		void connectionMade() override
		{
			// nothing to do at this point
			// we might want to bubble this up to the RemoteParameterServer
			// and offer a way to report how many connections exist
		}

		void connectionLost() override
		{
			// when the remote side disconnects then there is no more reason to live
			// When we go away the ParameterInterface will be freed which will do the right thing.
			_server->removeConnectionObject(this);

			// after calling removeConnectionObject we should have been deleted, so don't touch this!
		}

		void messageReceived (const juce::MemoryBlock& message) override
		{
			// remote side can tell us to change parameter values
			// parse the message and change the parameter via the ParameterInterface
			char *msg = (char *)message.getData();
			char objectname_attrname[512];
			double paramvalue = 0;
			int paramindex = -1;
			int verb;

			parseMessage(msg, objectname_attrname, verb, paramvalue);

			switch (verb) {
				case RNBO_PROBE_ASSIGN: {
					paramindex = _engine.getParameterIndexForID("probe_assign");
					if (paramindex != -1) {
						paramvalue = _engine.getSignalIndexForID(objectname_attrname);
						if (paramvalue != -1)
							_probeID = objectname_attrname;
						_probingSignalIndex = paramvalue;
						_parameterInterface->setParameterValue(paramindex, paramvalue);
						_probeParameterIndexRMS = _engine.getParameterIndexForID("probe_rms");
						_probeParameterIndexPeak = _engine.getParameterIndexForID("probe_peak");
					}
				}
					break;
				case RNBO_PARAM: {

					const Index numparams = _engine.getNumParameters();

					// here we could have made a map to speed getting the parameter name
					// but since we don't know when the names (code) change, we have to look it up
					// each time

					for (Index i = 0; i < numparams; i++) {
						if (!strcmp(objectname_attrname, _engine.getParameterId(i))) {
							paramindex = i;
							break;
						}
					}

					if (paramindex >= 0 && paramindex < numparams) {
						_parameterInterface->setParameterValue(paramindex, paramvalue);
					} else {
						// parameter index is out-of-range
						// this is not really a problem and could happen when editing via rnbo~ for example
					}
				}
					break;
				case RNBO_LISTEN_ON: {
					paramindex = _engine.getParameterIndexForID("listen_assign");
					if (paramindex != -1) {
						paramvalue = _engine.getSignalIndexForID(objectname_attrname);
						if (paramvalue != -1)
							_parameterInterface->setParameterValue(paramindex, paramvalue);
					}
				}
					break;
				case RNBO_LISTEN_OFF: {
					paramindex = _engine.getParameterIndexForID("listen_assign");
					if (paramindex != -1)
						_parameterInterface->setParameterValue(paramindex, -1);
				}
					break;
			}
		}

		// juce::AsyncUpdater
		void handleAsyncUpdate() override
		{
			drainEvents();
		}

		// EventHandler
		void eventsAvailable() override
		{
			triggerAsyncUpdate();
		}

		void handleParameterEvent(const ParameterEvent& pe) override
		{
			char msg[2048];
			Index index = pe.getIndex();
			const char *name = _engine.getParameterId(index);
			ParameterValue value = pe.getValue();
			ParameterInfo info;

			// Log() << "ClientConnectionHandler::handleParameterEvent: " << "index=" << pe.getParameterIndex() << "value=" << pe.getValue();

			_engine.getParameterInfo(index, &info);

			if (!info.transmittable)    // various assign parameters
				return;

			if (index == _probeParameterIndexPeak || index == _probeParameterIndexRMS) {
				if (_probingSignalIndex == -1)  // sorry, values no longer wanted by client
					return;
				sprintf(msg,"%s/%s %lf", _probeID.c_str(), name, value);
			} else
				sprintf(msg, "%s/param %lf", name, value);

			juce::MemoryBlock blk(msg, strlen(msg) + 1);
			sendMessage(blk);
		}

	private:

		void parseMessage(const char *msg, char *objname_attrname, int& verb, double &val)
		{
			const char *firstSlash = strchr(msg, '/');
			char verbstr[256] = "";

			// no slash ? no fun
			if (!firstSlash)
				return;

			// find the end of our parameter id and copy it
			const char *lastSlash = strrchr(msg, '/');
			if (!lastSlash)
				return;

			size_t num = lastSlash - firstSlash;
			objname_attrname[0] = 0;
			strncat(objname_attrname, firstSlash, num);

			// forget the last slash
			lastSlash++;;

			// find the space between verb and value
			const char * spaceBeforeVal = strchr(lastSlash, ' ');
			if (!spaceBeforeVal)
				return;

			// copy our verb
			num = spaceBeforeVal - lastSlash;
			strncat(verbstr, lastSlash, num);

			// forget the last space
			spaceBeforeVal++;

			auto result = _verbs.find(verbstr);
			if (result != _verbs.end())
				verb = result->second;
			else
				verb = RNBO_VERB_UNKNOWN;

			// our side doesn't care but for the freeze vector case you would need the
			// ability to get all N numbers that follow
			sscanf(spaceBeforeVal, "%lf", &val);
		}

		Engine& _engine;
		RemoteParameterServerImpl *_server;
		ParameterEventInterfaceUniquePtr _parameterInterface;
		String _probeID;
		Index _probeParameterIndexPeak;
		Index _probeParameterIndexRMS;
		Index _probingSignalIndex;
		std::map<String, int> _verbs;
	};

	juce::InterprocessConnection* RemoteParameterServerImpl::createConnectionObject()
	{
		std::unique_ptr<ClientConnectionHandler> cc = make_unique<ClientConnectionHandler>(_engine, this);
		juce::InterprocessConnection* ipcptr = cc.get();   // not beautiful that we give out pointer, but hopefully okay if managed properly
		_activeConnections.push_back(std::move(cc));
		return ipcptr;
	}

	void RemoteParameterServerImpl::removeConnectionObject(juce::InterprocessConnection* connection)
	{
		_activeConnections.erase(std::remove_if(_activeConnections.begin(),
												_activeConnections.end(),
												[connection](std::unique_ptr<ClientConnectionHandler>& cc) {
													return cc.get() == connection;
												}),
								 _activeConnections.end());
	}


}  // namespace RNBO

#endif // #ifdef USE_REMOTE_PARAMETER_CLIENT
