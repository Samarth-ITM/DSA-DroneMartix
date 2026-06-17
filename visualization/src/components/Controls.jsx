export default function Controls({ currentIndex, total, onNext, onReset, onRunDemo, isRunning }) {
  return (
    <div className="panel controls-panel">
      <div className="panel-header">
        <span className="panel-icon">⚙</span>
        <h2>Controls</h2>
      </div>

      <div className="stage-indicator">
        <span className="stage-label">State</span>
        <div className="stage-pips">
          {Array.from({ length: total }, (_, i) => (
            <div
              key={i}
              className={`stage-pip ${i === currentIndex ? 'pip-active' : i < currentIndex ? 'pip-done' : ''}`}
              title={`State ${i + 1}`}
            />
          ))}
        </div>
        <span className="stage-counter">{currentIndex + 1} / {total}</span>
      </div>

      <div className="controls-buttons">
        <button
          id="btn-run-demo"
          className="btn btn-primary"
          onClick={onRunDemo}
          disabled={isRunning}
          title="Auto-advance through all states"
        >
          {isRunning ? '⏸ Running...' : '▶ Run Demo'}
        </button>

        <button
          id="btn-next-tick"
          className="btn btn-secondary"
          onClick={onNext}
          disabled={currentIndex >= total - 1 || isRunning}
          title="Advance to next state"
        >
          ⏭ Next Tick
        </button>

        <button
          id="btn-reset"
          className="btn btn-outline"
          onClick={onReset}
          disabled={isRunning}
          title="Reset to initial state"
        >
          ↺ Reset
        </button>
      </div>

      <div className="controls-legend">
        <div className="legend-row">
          <span className="legend-dot" style={{ background: '#38a169' }}></span>
          <span>Battery ≥ 60%</span>
        </div>
        <div className="legend-row">
          <span className="legend-dot" style={{ background: '#d69e2e' }}></span>
          <span>Battery 30–59%</span>
        </div>
        <div className="legend-row">
          <span className="legend-dot" style={{ background: '#e53e3e' }}></span>
          <span>Battery &lt; 30%</span>
        </div>
        <div className="legend-row">
          <span className="legend-dot" style={{ background: '#f6c90e' }}></span>
          <span>Active Route</span>
        </div>
        <div className="legend-row">
          <span className="legend-dot" style={{ background: '#fc8181' }}></span>
          <span>Congested Corridor</span>
        </div>
      </div>
    </div>
  );
}
