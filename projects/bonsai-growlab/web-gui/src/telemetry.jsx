export function Telemetry({ data }) {
  if (!data) {
    return <p>Loading telemetry data...</p>;
  }

  return (
    <div>
      <h2>Telemetry</h2>
      <ul>
        {data &&
          Object.entries(data).map(([key, value]) => (
            <li key={key}>
              <strong>{key.replace(/_/g, " ")}:</strong> {value}
            </li>
          ))}
      </ul>
    </div>
  );
}
