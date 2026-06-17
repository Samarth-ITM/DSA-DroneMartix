import { useEffect, useRef } from 'react';

export default function EventLog({ state }) {
  const { events } = state;
  const listRef = useRef(null);

  useEffect(() => {
    if (listRef.current) {
      listRef.current.scrollTop = listRef.current.scrollHeight;
    }
  }, [events]);

  return (
    <div className="panel eventlog-panel">
      <div className="panel-header">
        <span className="panel-icon">📋</span>
        <h2>Event Log</h2>
      </div>
      <div ref={listRef} className="event-list">
        {events.map((evt, i) => {
          let cls = 'event-item';
          if (evt.includes('ALERT') || evt.includes('ERROR') || evt.includes('EMERGENCY')) cls += ' event-alert';
          else if (evt.includes('DELIVERED') || evt.includes('PASSED') || evt.includes('✔')) cls += ' event-success';
          else if (evt.includes('Congestion') || evt.includes('BLOCKED') || evt.includes('WAITING')) cls += ' event-warning';
          else if (evt.includes('Dijkstra') || evt.includes('Heap') || evt.includes('Stack')) cls += ' event-algo';

          return (
            <div key={i} className={cls}>
              <span className="event-index">{String(i + 1).padStart(2, '0')}</span>
              <span className="event-text">{evt}</span>
            </div>
          );
        })}
      </div>
    </div>
  );
}
