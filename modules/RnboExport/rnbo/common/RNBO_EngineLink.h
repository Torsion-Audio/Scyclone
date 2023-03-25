#ifndef _RNBO_EngineLink_h_
#define _RNBO_EngineLink_h_


namespace RNBO {

	class EngineInterface;
	class PatcherEventTarget;

	/**
	 * @private
	 */
	class EngineLink {

	public:

		void setEngineAndPatcher(EngineInterface *engineInterface, PatcherEventTarget* parentPatcher) {
			_engineInterface = engineInterface;
			_parentPatcher = parentPatcher;
		}

		virtual EngineInterface	*getEngine() const {
			return _engineInterface;
		}

		PatcherEventTarget* getPatcherEventTarget() const {
			return _parentPatcher;
		}

	protected:

		~EngineLink() { }

		EngineInterface*	_engineInterface = nullptr;
		PatcherEventTarget*	_parentPatcher = nullptr;
	};

} // namespace RNBO

#endif // #ifndef _RNBO_EngineLink_h_
