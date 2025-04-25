const int trigPin = 19;    // GPIO pin connected to Ultrasonic Trigger pin
const int echoPin = 21;     // GPIO pin connected to Ultrasonic Echo pin
const int ledPin = 18;     // GPIO pin connected to LED
const int ledOnTime = 10000; // Time LED stays on after detection (in milliseconds)
const float maxDistance = 20.0; // Max detection distance in cm (4 feet ≈ 122 cm)

unsigned long motionDetectedTime = 0; // Time when object was last detected
bool ledOn = false;       // LED state

void setup() {
  pinMode(trigPin, OUTPUT); // Set Trigger pin as output
  pinMode(echoPin, INPUT);  // Set Echo pin as input
  pinMode(ledPin, OUTPUT);  // Set LED pin as output
  digitalWrite(ledPin, LOW); // Ensure LED is off initially
  Serial.begin(115200);     // Start serial communication for debugging
}

void loop() {
  // Send 10µs pulse to Trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure duration of Echo pulse
  long duration = pulseIn(echoPin, HIGH);

  // Calculate distance in cm (speed of sound = 0.0343 cm/µs, divide by 2 for round trip)
  float distance = duration * 0.0343 / 2;

  // Check if object is within 4 feet
  if (distance <= maxDistance && distance > 0) {
    Serial.print("Object detected at ");
    Serial.print(distance);
    Serial.println(" cm");
    digitalWrite(ledPin, HIGH); // Turn on LED
    ledOn = true;
    motionDetectedTime = millis(); // Record time of detection
  }

  // Turn off LED after ledOnTime if no new detection
  if (ledOn && (millis() - motionDetectedTime >= ledOnTime)) {
    digitalWrite(ledPin, LOW); // Turn off LED
    ledOn = false;
    Serial.println("LED turned off.");
  }

  delay(100); // Small delay to prevent excessive loop speed
}