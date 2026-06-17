import { NODES } from '../data/simulationStates.js';

function getNodeName(id) {
  const node = NODES.find(n => n.id === id);
  return node ? node.name.replace(/_/g, ' ') : `Node ${id}`;
}

function getMissionStatusClass(status) {
  switch (status) {
    case 'ACTIVE':     return 'mission-active';
    case 'RETURNING':  return 'mission-returning';
    case 'EMERGENCY':  return 'mission-emergency';
    case 'WAITING':    return 'mission-waiting';
    case 'ARCHIVED':   return 'mission-archived';
    case 'BLOCKED':    return 'mission-blocked';
    default:           return '';
  }
}

export default function MissionPanel({ state }) {
  const { missions } = state;

  if (missions.length === 0) {
    return (
      <div className="panel mission-panel">
        <div className="panel-header">
          <span className="panel-icon">🎯</span>
          <h2>Mission Panel</h2>
        </div>
        <p className="empty-state">No active missions.</p>
      </div>
    );
  }

  return (
    <div className="panel mission-panel">
      <div className="panel-header">
        <span className="panel-icon">🎯</span>
        <h2>Mission Panel</h2>
      </div>
      <div className="mission-list">
        {missions.map(m => {
          const progressPct = m.route.length > 1
            ? Math.round((m.progress / (m.route.length - 1)) * 100)
            : 100;

          return (
            <div key={m.id} className={`mission-card ${getMissionStatusClass(m.status)}`}>
              <div className="mission-header-row">
                <span className="mission-id">Mission {m.id}</span>
                <span className={`badge ${getMissionStatusClass(m.status)}`}>{m.status}</span>
              </div>
              <div className="mission-meta-row">
                <div className="mission-meta">
                  <span className="meta-label">Drone</span>
                  <span className="meta-value">{m.droneId}</span>
                </div>
                {m.packageId && (
                  <div className="mission-meta">
                    <span className="meta-label">Package</span>
                    <span className="meta-value">#{m.packageId}</span>
                  </div>
                )}
                <div className="mission-meta">
                  <span className="meta-label">Dest</span>
                  <span className="meta-value">{getNodeName(m.dest)}</span>
                </div>
              </div>
              <div className="mission-route">
                <span className="meta-label">Route: </span>
                {m.route.map((nodeId, i) => (
                  <span key={i} className={`route-node ${i === m.progress ? 'route-current' : i < m.progress ? 'route-done' : ''}`}>
                    {i > 0 && <span className="route-arrow">→</span>}
                    {nodeId}
                  </span>
                ))}
              </div>
              <div className="mission-progress-row">
                <div className="progress-bar-bg">
                  <div
                    className="progress-bar-fill"
                    style={{ width: `${progressPct}%` }}
                  />
                </div>
                <span className="progress-label">{m.progress}/{m.route.length - 1}</span>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}
