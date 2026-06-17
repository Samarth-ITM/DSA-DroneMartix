function getBatteryColor(pct) {
  if (pct >= 60) return '#38a169';
  if (pct >= 30) return '#d69e2e';
  return '#e53e3e';
}

function getStatusBadgeClass(status) {
  const s = status.toLowerCase();
  if (s.includes('deliver') || s.includes('active')) return 'badge badge-active';
  if (s.includes('return')) return 'badge badge-returning';
  if (s.includes('idle'))   return 'badge badge-idle';
  if (s.includes('emergency') || s.includes('low')) return 'badge badge-emergency';
  if (s.includes('wait') || s.includes('block'))    return 'badge badge-warning';
  if (s.includes('charg') || s.includes('selected')) return 'badge badge-charging';
  return 'badge badge-idle';
}

export default function FleetDashboard({ state }) {
  const { drones } = state;

  return (
    <div className="panel fleet-panel">
      <div className="panel-header">
        <span className="panel-icon">🛰</span>
        <h2>Fleet Dashboard</h2>
      </div>
      <div className="fleet-grid">
        {drones.map(drone => {
          const battColor = getBatteryColor(drone.battery);
          return (
            <div key={drone.id} className="drone-card">
              <div className="drone-card-header">
                <span className="drone-id">Drone {drone.id}</span>
                <span className={getStatusBadgeClass(drone.status)}>{drone.status}</span>
              </div>
              <div className="battery-row">
                <div className="battery-bar-bg">
                  <div
                    className="battery-bar-fill"
                    style={{ width: `${Math.max(drone.battery, 0)}%`, background: battColor }}
                  />
                </div>
                <span className="battery-label" style={{ color: battColor }}>
                  {drone.battery}%
                </span>
              </div>
              <div className="drone-meta">
                <span className="meta-label">Node</span>
                <span className="meta-value">{drone.node}</span>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}
