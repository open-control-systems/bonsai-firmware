import { useEffect, useState } from "preact/hooks";
import "./app.css";
import { Telemetry } from "./telemetry.jsx";
import { Registration } from "./registration.jsx";

// Base URL.
const API_BASE_URL = "api/v1";

// Default interval to fetch the telemetry data, 10 seconds.
const TELEMETRY_FETCH_INTERVAL = 10 * 1000;

// Default interval to fetch the registration data, 1 minute.
const REGISTRATION_FETCH_INTERVAL = 60 * 1000;

export function App() {
  const [telemetry, setTelemetry] = useState(null);
  const [registration, setRegistration] = useState(null);

  // Fetch telemetry data periodically
  useEffect(() => {
    const fetchTelemetry = async () => {
      try {
        const response = await fetch(`${API_BASE_URL}/telemetry`);
        if (response.ok) {
          const data = await response.json();
          setTelemetry(data);
        } else {
          console.error("Failed to fetch telemetry:", response.statusText);
        }
      } catch (error) {
        console.error("Error fetching telemetry:", error);
      }
    };

    const fetchRegistration = async () => {
      try {
        const response = await fetch(`${API_BASE_URL}/registration`);
        if (response.ok) {
          const data = await response.json();
          setRegistration(data);
        } else {
          console.error("Failed to fetch registration:", response.statusText);
        }
      } catch (error) {
        console.error("Error fetching registration:", error);
      }
    };

    fetchTelemetry();
    fetchRegistration();

    // Set intervals
    const telemetryIntervalId = setInterval(
      fetchTelemetry,
      TELEMETRY_FETCH_INTERVAL,
    );
    const registrationIntervalId = setInterval(
      fetchRegistration,
      REGISTRATION_FETCH_INTERVAL,
    );

    // Cleanup intervals on unmount
    return () => {
      clearInterval(telemetryIntervalId);
      clearInterval(registrationIntervalId);
    };
  }, []);

  return (
    <div style={{ padding: "20px", fontFamily: "Arial, sans-serif" }}>
      <h1>Bonsai Growlab Dashboard</h1>
      <Registration data={registration} />
      <Telemetry data={telemetry} />
    </div>
  );
}
