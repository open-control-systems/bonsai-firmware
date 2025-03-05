export function Registration({ data }) {
  if (!data) {
    return <p>Loading registration data...</p>;
  }

  return (
    <div>
      <h2>Registration</h2>
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
