function getPkgStatusClass(status) {
  switch (status) {
    case 'DELIVERED':   return 'pkg-delivered';
    case 'IN_TRANSIT':  return 'pkg-transit';
    case 'DISPATCHED':  return 'pkg-dispatched';
    case 'RESOLVING':   return 'pkg-resolving';
    case 'BLOCKED':     return 'pkg-blocked';
    default:            return 'pkg-pending';
  }
}

function getPkgIcon(status) {
  switch (status) {
    case 'DELIVERED':  return '✓';
    case 'IN_TRANSIT': return '→';
    case 'BLOCKED':    return '✕';
    case 'RESOLVING':  return '⏳';
    default:           return '□';
  }
}

export default function PackageQueue({ state }) {
  const { packages } = state;

  return (
    <div className="panel queue-panel">
      <div className="panel-header">
        <span className="panel-icon">📦</span>
        <h2>Package Queue (FIFO)</h2>
      </div>
      <div className="queue-list">
        {packages.map((pkg, index) => (
          <div key={pkg.id} className={`queue-item ${getPkgStatusClass(pkg.status)}`}>
            <span className="queue-position">#{index + 1}</span>
            <span className="pkg-icon">{getPkgIcon(pkg.status)}</span>
            <div className="pkg-info">
              <span className="pkg-id">PKG #{pkg.id}</span>
              <span className="pkg-dest">{pkg.dest}</span>
            </div>
            <span className={`pkg-badge ${getPkgStatusClass(pkg.status)}`}>
              {pkg.status}
            </span>
          </div>
        ))}
      </div>
    </div>
  );
}
