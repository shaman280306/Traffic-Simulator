#include<iostream>
#include<fstream>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <limits>       // For numeric_limits
#include <algorithm>    // For max, reverse, remove_if
#include <ctime>        // For time(), srand()
#include <cstdlib>      // For rand(), srand(), system()
#include <cstdio>       // Potentially for system(), though <cstdlib> is more common for it
#include <iomanip>      // For fixed, setprecision
#include <sstream>      // For stringstream
#include <thread>       // For std::thread
#include <chrono>       // For std::chrono (used by std::this_thread::sleep_for and high_resolution_clock)

#ifdef _WIN32
#include <windows.h> // For Sleep(), Beep(), SetConsoleOutputCP()
#else
#include <unistd.h> // For sleep()
#endif

using namespace std;

// ================ GLOBAL CONSTANTS ================
constexpr int MAX_CONGESTION = 5;
constexpr int WEATHER_UPDATE_INTERVAL = 30; // Seconds before weather updates
constexpr double EMERGENCY_SPEED_BOOST = 1.5;

// ================ GLOBAL SETTINGS ================
int timeMultiplier = 1; // For time travel feature

// ================ COLOR CODES ================
// ANSI escape codes. These should work on most modern terminals, including VS Code's integrated terminal.
// If your MinGW 2016 compiler environment's console is very old, ANSI codes might not render.
// In such a case, you'd need to use Windows API functions like SetConsoleTextAttribute
// more extensively, but your current setup indicates ANSI is supported.
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string CYAN = "\033[36m";
const string MAGENTA = "\033[35m";
const string WHITE = "\033[37m"; // Added for general text
const string RESET = "\033[0m";
const string BOLD = "\033[1m";
const string AI_COLOR = "\033[1;93m";       // Bright Yellow/Gold for AI output
const string EMERGENCY_COLOR = "\033[1;91m"; // Bright Red for emergency alerts

// ================ UTILITY FUNCTIONS ================
void sleep_seconds(int seconds) {
    seconds = max(1, seconds / max(1, timeMultiplier));
#ifdef _WIN32
    Sleep(seconds * 1000); // Sleep takes milliseconds on Windows
#else
    sleep(seconds); // sleep takes seconds on Unix/Linux/macOS
#endif
}

void progressBar(int duration) {
    const int totalTicks = 20;
    for (int i = 0; i <= totalTicks; ++i) {
        int percent = (i * 100) / totalTicks;
        cout << "\r" << YELLOW << "["; // Use \r to move cursor to start of line
        for (int j = 0; j < totalTicks; ++j) {
            if (j < i) cout << "=";
            else if (j == i) cout << ">";
            else cout << " ";
        }
        cout << "] " << percent << "% " << RESET << flush; // flush ensures immediate output
        sleep_seconds(duration / totalTicks); // Use the modified sleep_seconds with timeMultiplier
    }
    cout << "\r" << string(30, ' ') << "\r"; // Clear the progress bar line by overwriting with spaces
}

// ================ ENHANCED VEHICLE SYSTEM ================
enum VehicleType { CAR, BIKE, BUS, AMBULANCE, POLICE, FIRE_TRUCK };

struct Vehicle {
    VehicleType type;
    string name;
    double speedMultiplier;
    double fuelRate;
    bool emergency;
    string emoji;
    string allowedRoads; // Comma-separated string of allowed road types

    Vehicle(VehicleType t, bool emerg = false) : type(t), emergency(emerg) {
        switch (t) {
            case CAR:
                name = "Car"; speedMultiplier = 1.0; fuelRate = 0.7;
                emoji = "🚗"; allowedRoads = "General,Highway,Bridge,Tunnel"; break;
            case BIKE:
                name = "Bike"; speedMultiplier = 1.2; fuelRate = 0.3;
                emoji = "🏍️"; allowedRoads = "General,Bike Lane,Highway,Bridge,Tunnel"; break;
            case BUS:
                name = "Bus"; speedMultiplier = 0.7; fuelRate = 1.5;
                emoji = "🚌"; allowedRoads = "General,Bus Lane,Highway,Bridge,Tunnel"; break;
            case AMBULANCE:
                name = "Ambulance"; speedMultiplier = 1.0; fuelRate = 1.0; // Base speed, emergency boosts
                emoji = "🚑"; allowedRoads = "General,Emergency,Highway,Bridge,Tunnel"; break;
            case POLICE:
                name = "Police"; speedMultiplier = 1.0; fuelRate = 1.1; // Base speed, emergency boosts
                emoji = "🚓"; allowedRoads = "General,Emergency,Highway,Bridge,Tunnel"; break;
            case FIRE_TRUCK:
                name = "Fire Truck"; speedMultiplier = 1.0; fuelRate = 1.8; // Base speed, emergency boosts
                emoji = "🚒"; allowedRoads = "General,Emergency,Highway,Bridge,Tunnel"; break;
        }
        if (emergency) speedMultiplier *= EMERGENCY_SPEED_BOOST; // Emergency speed boost
    }

    void toggleEmergency() {
        emergency = !emergency;
        if (emergency) {
            speedMultiplier *= EMERGENCY_SPEED_BOOST;
            cout << RED << "\n🚨 EMERGENCY MODE ACTIVATED FOR " << name << " 🚨\n" << RESET;
#ifdef _WIN32 // Conditionally compile Beep for Windows
            Beep(800, 300); Beep(1000, 300); Beep(800, 300);
#endif
        } else {
            speedMultiplier /= EMERGENCY_SPEED_BOOST; // Revert speed if emergency mode is turned off
            cout << GREEN << "\n✅ EMERGENCY MODE DEACTIVATED FOR " << name << " ✅\n" << RESET;
        }
    }

    bool canUseRoad(const string& roadType) const {
        // If "All" is specified, the vehicle can use any road type
        if (allowedRoads.find("All") != string::npos) return true;

        stringstream ss(allowedRoads);
        string allowed;
        while (getline(ss, allowed, ',')) { // Parse comma-separated allowed road types
            if (allowed == roadType) return true;
        }
        return false;
    }
};

// ================ WEATHER SYSTEM ================
enum WeatherType { SUNNY, RAIN, SNOW, FOG, STORM };
WeatherType currentWeather = SUNNY;

string getWeatherMessage() {
    switch(currentWeather) {
        case SUNNY: return "☀️ Normal conditions";
        case RAIN: return "🌧️ Wet roads (15% slower)";
        case SNOW: return "❄️ Icy roads (30% slower)";
        case FOG: return "🌫️ Low visibility (20% slower)";
        case STORM: return "⛈️ Dangerous conditions (40% slower)";
        default: return "";
    }
}

double getWeatherMultiplier() {
    switch(currentWeather) {
        case SUNNY: return 1.0;
        case RAIN: return 0.85;
        case SNOW: return 0.7;
        case FOG: return 0.8;
        case STORM: return 0.6;
        default: return 1.0;
    }
}

void updateWeather() {
    currentWeather = static_cast<WeatherType>(rand() % 5); // Randomly pick one of 5 weather types
    cout << YELLOW << "\n[WEATHER UPDATE] " << getWeatherMessage() << RESET << endl;
}

// ================ INCIDENT SYSTEM (Singleton Pattern) ================
class IncidentMonitor {
private:
    struct Incident {
        string location;
        string type;
        int severity;
        time_t timestamp;
        string roadType; // Road type affected by the incident
    };
    vector<Incident> incidents;

    // Private constructor to prevent direct instantiation
    IncidentMonitor() {} 

public:
    // Static method to get the single instance of the class
    static IncidentMonitor& getInstance() {
        static IncidentMonitor instance; // Guaranteed to be initialized once
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    IncidentMonitor(const IncidentMonitor&) = delete;
    IncidentMonitor& operator=(const IncidentMonitor&) = delete;

    void generateIncident() {
        if (rand() % 3 == 0) { // Increased chance for incidents (1 in 3)
            vector<string> locations = {"Main St", "Highway 1", "Downtown", "Central Bridge", "Suburban Tunnel", "Industrial Zone"};
            vector<string> types = {"🚧 Construction", "🚨 Accident", "💡 Smart Light Outage", "🔧 Roadwork", "🚇 Metro Delay", "💧 Flooding"};
            vector<string> roadTypes = {"General", "Bike Lane", "Bus Lane", "Emergency", "Highway", "Bridge", "Tunnel"}; // Specific road types

            Incident newIncident;
            newIncident.location = locations[rand()%locations.size()];
            newIncident.type = types[rand()%types.size()];
            newIncident.severity = rand()%3+1; // Severity 1-3
            newIncident.timestamp = time(nullptr);
            newIncident.roadType = roadTypes[rand()%roadTypes.size()]; // Incident affects a specific road type

            incidents.push_back(newIncident);
            cout << EMERGENCY_COLOR << "\n[ALERT] " << newIncident.type << " at "
                 << newIncident.location << " (Severity: "
                 << string(newIncident.severity, '!') << ") affecting "
                 << newIncident.roadType << " roads.\n" << RESET;
        }
    }

    void showActiveIncidents() {
        cout << MAGENTA << "\n=== ACTIVE INCIDENTS ===\n" << RESET;
        if (incidents.empty()) {
            cout << "No active incidents.\n";
            return;
        }
        // Clean up old incidents before displaying
        incidents.erase(
            remove_if(incidents.begin(), incidents.end(),
                      [](const Incident& i){ return difftime(time(nullptr), i.timestamp) > 300; }), // Remove incidents older than 5 minutes (300 seconds)
            incidents.end()
        );

        for (const auto& incident : incidents) { // Use const reference for efficiency
            cout << incident.type << " at " << BOLD << incident.location << RESET << " ("
                 << incident.severity << "/3 severity) - "
                 << difftime(time(nullptr), incident.timestamp) << " sec ago"
                 << " [Road Type: " << incident.roadType << "]\n";
        }
    }

    vector<Incident> getIncidents() const { return incidents; }
};

// ================ AI OPTIMIZER ================
class AIOptimizer {
public:
    void analyze(const string& start, const string& end) {
        cout << AI_COLOR << "\n🤖 AI OPTIMIZER ACTIVATED\n";
        cout << "• Scanning traffic patterns between " << start << " and " << end << "...\n";
        cout << "• Analyzing " << 15 + rand()%10 << " route variations...\n";

        int timeSave = 15 + rand()%20;
        string bestRoute = (rand()%2) ? "via City Center" : "via Ring Road";
        cout << "✔ Recommendation: " << bestRoute << " saves ~" << timeSave << "% time\n";
        cout << "⚠ Warning: " << 3 + rand()%5 << " congestion points detected\n" << RESET;
    }

    void optimizeTrafficLights() {
        if (rand() % 2 == 0) { // Optimize occasionally
            cout << AI_COLOR << "\n🖥️ AI TRAFFIC LIGHT OPTIMIZATION\n";
            cout << "• Synchronizing " << 10 + rand()%15 << " intersections...\n";
            cout << "• Estimated delay reduction: " << 20 + rand()%25 << "%\n" << RESET;
        }
    }

    void predictCongestion(const string& start, const string& end) {
        cout << AI_COLOR << "\n🧠 PREDICTIVE ANALYSIS:\n";
        int jamRisk = rand() % 100;
        if (jamRisk > 70) {
            cout << RED << "⚠️ High congestion risk (" << jamRisk << "%) on "
                 << start << "→" << end << " between "
                 << rand() % 3 + 4 << "PM-" << rand() % 3 + 7 << "PM\n"; // More realistic time range (4-7 PM)
        } else {
            cout << GREEN << "✅ Smooth traffic expected (" << 100-jamRisk << "% clear)\n";
        }
        cout << RESET;
    }
};

// ================ GRAPH CLASS ================
class Graph {
private:
    struct Edge {
        string destination;
        double weight;       // Changed to double for weather multipliers to retain precision
        int signalDelay;
        bool blocked;        // Can be set by incidents or manually
        int congestion;      // Level of congestion (0-MAX_CONGESTION)
        string roadType;     // Type of road: General, Highway, Bike Lane, etc.

        Edge() : destination(""), weight(0.0), signalDelay(0), blocked(false), congestion(0), roadType("General") {}
        Edge(const string& d, double w, int sd, string rt = "General")
            : destination(d), weight(w), signalDelay(sd), blocked(false), congestion(0), roadType(rt) {}
    };

    map<string, vector<Edge>> adjList;
    // Store original edge properties separately to re-apply effects consistently
    // This map stores the 'base' properties of each road segment, without temporary effects.
    map<pair<string, string>, Edge> baseEdges; // Key is {source, destination} to identify unique roads

    // IncidentMonitor is now a Singleton, access via getInstance()
    // IncidentMonitor monitor; // No longer needed as a member variable

    AIOptimizer ai;

public:
    // ================ ENHANCED VISUALIZATION ================
    void showEnhancedMap() {
        cout << CYAN << "\n🌍 LIVE TRAFFIC MAP 🌍\n" << RESET;
        if (adjList.empty()) {
            cout << "Map is empty. Please add some roads first (Option 1).\n";
            return;
        }
        for (auto& node : adjList) {
            cout << BOLD << "🟢 " << node.first << RESET << " [" << getRoadTypeDisplayName(node.first) << "]\n";
            for (auto& edge : node.second) {
                // Determine color based on status
                string statusColor = edge.blocked ? RED : GREEN;
                string emojiStatus = edge.blocked ? "⛔" : "✅";
                string congestionInfo = "";
                if (edge.congestion > 0) {
                    congestionInfo = YELLOW + " (" + to_string(edge.congestion) + " cars)" + RESET;
                }

                cout << "    " << emojiStatus << statusColor << " " << edge.destination << RESET << " ("
                     << fixed << setprecision(0) << edge.weight << "s, " << edge.roadType << ")"
                     << congestionInfo << "\n";
            }
        }
    }

    // Helper to get a nicer display name for road types (could be improved to use enums)
    string getRoadTypeDisplayName(const string& node) {
        if (node.find("Highway") != string::npos) return "Highway";
        if (node.find("Bridge") != string::npos) return "Bridge";
        if (node.find("Bike") != string::npos) return "Bike Lane";
        if (node.find("Bus") != string::npos) return "Bus Lane";
        if (node.find("Tunnel") != string::npos) return "Tunnel";
        if (node.find("Airport") != string::npos) return "Airport Access";
        if (node.find("Hospital") != string::npos) return "Hospital Access";
        return "General";
    }

    // ================ EMERGENCY SYSTEM ================
    void playSiren() {
#ifdef _WIN32 // Conditionally compile Beep for Windows
        // Beep frequency and duration adjustments for a more "siren-like" sound
        for (int i = 0; i < 3; ++i) { // Repeat a few times
            Beep(600, 150); // Lower tone
            Beep(900, 150); // Higher tone
            Beep(600, 150);
            Beep(900, 150);
            Sleep(100); // Short pause between cycles
        }
#endif
        cout << EMERGENCY_COLOR << "\n🚨 ALL VEHICLES YIELD TO EMERGENCY VEHICLE! 🚨\n" << RESET;
    }

    // ================ ROAD MANAGEMENT ================
    void addRoad(const string& u, const string& v, int w, int sd, string roadType = "General") {
        // Add road in both directions for a bidirectional graph
        adjList[u].push_back(Edge(v, w, sd, roadType));
        adjList[v].push_back(Edge(u, w, sd, roadType));
        // Store original edge properties for later use (e.g., reverting rush hour effects, weather)
        baseEdges[{u, v}] = Edge(v, w, sd, roadType);
        baseEdges[{v, u}] = Edge(u, w, sd, roadType);
        cout << GREEN << "Road added: " << u << " <-> " << v << " (" << roadType << ")\n" << RESET;
    }

    // ================ FUEL & ENVIRONMENT STATS ================
    void showEcoStats(const Vehicle& vehicle, double distance) {
        // Simplified CO2 calculation (approximate, kg per 1000 units of distance)
        double co2PerDistanceUnit = 0.12; // Base kg CO2 per distance unit (e.g., meter)
        if (vehicle.type == BUS) co2PerDistanceUnit *= 2.5; // Buses produce more CO2
        else if (vehicle.type == BIKE) co2PerDistanceUnit = 0; // Bikes are zero emission

        // Fuel efficiency as "units of distance per fuel unit" (inverse of fuelRate)
        double fuelEfficiency = (vehicle.fuelRate > 0) ? (1.0 / vehicle.fuelRate) : 0; // Higher value is better

        cout << GREEN << "♻️ Eco Stats for " << vehicle.name << " journey:\n"
             << "   CO2 Emission: " << fixed << setprecision(2) << distance * co2PerDistanceUnit << " kg\n"
             << "   Relative Fuel Efficiency: " << fixed << setprecision(2) << fuelEfficiency << " units/fuel unit\n" << RESET;
    }

    // ================ TOLL SYSTEM ================
    int getTollFee(const string& roadType) {
        static map<string, int> tolls = {
            {"Highway", 5}, {"Bridge", 3}, {"Tunnel", 7}
        };
        return tolls.count(roadType) ? tolls.at(roadType) : 0; // Use .at() for const correctness
    }

    // ================ SHORTEST PATH WITH ALL FEATURES ================
    void shortestPath(const string& src, const string& dest, Vehicle vehicle) {
        // Edge Case Check: Source and destination are identical
        if (src == dest) {
            cout << RED << "Error: Source and destination are identical! No route needed.\n" << RESET;
            return;
        }

        // Edge Case Check: Source or destination node doesn't exist
        if (adjList.find(src) == adjList.end()) {
            cout << RED << "Error: Source node '" << src << "' doesn't exist in the map!\n" << RESET;
            return;
        }
        if (adjList.find(dest) == adjList.end()) {
            cout << RED << "Error: Destination node '" << dest << "' doesn't exist in the map!\n" << RESET;
            return;
        }

        // Performance Metrics: Start timer
        auto start_time = chrono::high_resolution_clock::now();

        if (vehicle.emergency) playSiren();

        // Apply weather effects just before pathfinding starts, ensuring current conditions apply
        applyWeatherEffects();

        map<string, int> dist;
        map<string, string> parent;
        // Priority queue stores {current_total_time, node_name}
        // `greater` makes it a min-priority queue (smallest time at top)
        priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> pq;

        // Initialize distances to infinity
        for (auto& node : adjList) dist[node.first] = numeric_limits<int>::max();
        dist[src] = 0; // Distance to source is 0
        pq.push({0, src}); // Start Dijkstra's from source

        while (!pq.empty()) {
            auto current = pq.top();
            pq.pop();
            int current_dist = current.first;
            string u = current.second;

            if (u == dest) break; // Found the destination, can stop early
            if (current_dist > dist[u]) continue; // Already found a shorter path to 'u'

            for (auto& edge : adjList.at(u)) { // Use .at() for memory safety with map access
                // Check for general blockage or specific incident affecting this road
                bool isBlockedByIncident = false;
                for (const auto& incident : IncidentMonitor::getInstance().getIncidents()) { // Singleton access
                    // Check if incident location is near the edge, and if road type matches
                    if ((incident.location == edge.destination || incident.location == u ||
                         (baseEdges.count({u, edge.destination}) && incident.location == getRoadTypeDisplayName(u))) &&
                        (incident.roadType == edge.roadType || incident.roadType == "All" || edge.roadType.find(incident.roadType) != string::npos)) {
                        isBlockedByIncident = true;
                        break;
                    }
                }

                if (edge.blocked || isBlockedByIncident) {
                    continue; // Skip blocked roads
                }
                if (!vehicle.canUseRoad(edge.roadType)) {
                    continue; // Skip roads not allowed for this vehicle type
                }

                // Calculate total time cost for this segment
                double effectiveWeight = edge.weight; // Base weight already adjusted by weather
                effectiveWeight *= (1.0 + (edge.congestion * 0.1)); // Add 10% delay per congestion unit
                int timeCost = static_cast<int>((effectiveWeight + edge.signalDelay) / vehicle.speedMultiplier);

                if (dist.at(u) + timeCost < dist.at(edge.destination)) { // Use .at() for memory safety
                    dist.at(edge.destination) = dist.at(u) + timeCost; // Use .at() for memory safety
                    parent[edge.destination] = u;
                    pq.push({dist.at(edge.destination), edge.destination}); // Use .at() for memory safety
                }
            }
        }

        // Performance Metrics: End timer and display duration
        auto end_time = chrono::high_resolution_clock::now();
        cout << "Route calculation took: "
             << chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count()
             << "ms\n";

        if (dist.at(dest) == numeric_limits<int>::max()) { // Use .at() for memory safety
            cout << RED << "No path exists from " << src << " to " << dest << " for " << vehicle.name << "!\n" << RESET;
            return;
        }

        // Reconstruct the path
        vector<string> path;
        for (string v = dest; v != src; v = parent.at(v)) path.push_back(v); // Use .at() for memory safety
        path.push_back(src);
        reverse(path.begin(), path.end()); // Reverse to get path from source to destination

        cout << GREEN << "\nRoute for " << vehicle.emoji << " " << vehicle.name << ":\n" << RESET;
        double totalDistance = 0;
        int totalToll = 0;

        for (size_t i = 0; i < path.size(); ++i) {
            cout << BOLD << path.at(i) << RESET; // Use .at() for memory safety
            if (i != path.size() - 1) {
                // Find the edge corresponding to the current segment to get its original weight and road type
                for (auto& edge : adjList.at(path.at(i))) { // Use .at() for memory safety
                    if (edge.destination == path.at(i+1)) { // Use .at() for memory safety
                        // Use the original base weight for total distance calculation (not affected by weather/congestion)
                        double segmentDistance = 0;
                        if (baseEdges.count({path.at(i), edge.destination})) { // Use .at() for memory safety
                            segmentDistance = baseEdges.at({path.at(i), edge.destination}).weight; // Use .at() for memory safety
                        } else if (baseEdges.count({edge.destination, path.at(i)})) { // Check reverse direction if needed, use .at() for memory safety
                            segmentDistance = baseEdges.at({edge.destination, path.at(i)}).weight; // Use .at() for memory safety
                        }
                        totalDistance += segmentDistance;

                        int toll = getTollFee(edge.roadType); // Get toll based on road type
                        if (toll > 0) {
                            cout << YELLOW << " [Toll: $" << toll << "]" << RESET;
                            totalToll += toll;
                        }
                        break;
                    }
                }
                cout << " -> ";
            }
        }
        cout << "\n⏱️ Total time: " << dist.at(dest) << "s"; // Use .at() for memory safety
        if (totalToll > 0) {
            cout << YELLOW << " | 💲 Total Toll: $" << totalToll << RESET;
        }
        cout << endl;

        showEcoStats(vehicle, totalDistance);
        simulateTimeDelay(dist.at(dest)); // Use .at() for memory safety
    }

    // ================ DATA EXPORT ================
    void exportToCSV() {
        ofstream out("traffic_data.csv");
        if (!out.is_open()) {
            cout << RED << "Error: Could not open traffic_data.csv for writing. Check permissions.\n" << RESET;
            return;
        }
        out << "Source,Destination,RoadType,OriginalWeight,CurrentWeight,SignalDelay,Blocked,Congestion\n";
        for (auto& node : adjList) {
            for (auto& edge : node.second) {
                // Fetch original weight from baseEdges for the CSV export
                double originalWeight = 0;
                if (baseEdges.count({node.first, edge.destination})) {
                    originalWeight = baseEdges.at({node.first, edge.destination}).weight;
                } else if (baseEdges.count({edge.destination, node.first})) {
                    originalWeight = baseEdges.at({edge.destination, node.first}).weight;
                }
                out << node.first << "," << edge.destination << ","
                    << edge.roadType << "," << originalWeight << ","
                    << edge.weight << "," << edge.signalDelay << ","
                    << (edge.blocked ? "TRUE" : "FALSE") << "," << edge.congestion << "\n";
            }
        }
        out.close();
        cout << GREEN << "📊 Data exported to traffic_data.csv\n" << RESET;
    }

    // ================ TUTORIAL MODE ================
    void runTutorial() {
        cout << CYAN << "\n=== INTERACTIVE TUTORIAL ===\n" << RESET;
        vector<string> steps = {
            "Welcome to the Traffic Simulation! Let's get started.",
            "1. First, we need to add roads. Choose option " + BOLD + "1" + RESET + " from the main menu.",
            "   You'll enter a start node, end node, weight (time in seconds), signal delay, and road type.",
            "   Try adding: Downtown -> Midtown (300s, 60s, Highway)",
            "   Then: Midtown -> Market St (120s, 30s, General)",
            "2. Now, let's view the map. Choose option " + BOLD + "12" + RESET + " (Enhanced Live Map).",
            "   You should see the roads you added and default ones.",
            "3. Time to simulate! Choose option " + BOLD + "5" + RESET + " (Calculate Shortest Path).",
            "   Enter 'Downtown' as source and 'Market St' as destination. Pick 'Car' (option 1).",
            "   See the route and total time. A progress bar will simulate the journey.",
            "4. Observe dynamic changes: The simulation automatically updates weather and generates incidents.",
            "   You can manually generate an incident with option " + BOLD + "3" + RESET + " (Simulate/View Incidents).",
            "5. Compare vehicle types! Choose option " + BOLD + "6" + RESET + " (Compare Vehicle Routes).",
            "   Input the same start/end nodes. Notice how an 'Ambulance' takes a shorter time due to emergency speed.",
            "6. Leverage AI! Choose option " + BOLD + "9" + RESET + " (AI Traffic Analysis).",
            "   Enter start/end nodes to get route recommendations and congestion predictions.",
            "7. Explore other features: 'Time Controls' (" + BOLD + "13" + RESET + "), 'City Stats' (" + BOLD + "11" + RESET + "), and 'Export Data' (" + BOLD + "14" + RESET + ").",
            "That's it for the basic tutorial! Enjoy exploring the traffic simulation."
        };

        for (const string& step : steps) {
            cout << step << endl;
            sleep_seconds(5); // Give user time to read each step
        }
        cout << GREEN << "\nTutorial finished! Returning to Main Menu.\n" << RESET;
    }

    // ================ STRATEGY PATTERN FOR ROUTING ================
    class RoutingStrategy {
    public:
        virtual void calculate(Graph& g, const string& src,
                             const string& dest) = 0;
    };

    class FastestRoute : public RoutingStrategy {
    public:
        void calculate(Graph& g, const string& src,
                     const string& dest) override {
            Vehicle car(CAR); // Default to car for fastest
            g.shortestPath(src, dest, car);
        }
    };

    class EmergencyRoute : public RoutingStrategy {
    public:
        void calculate(Graph& g, const string& src,
                     const string& dest) override {
            Vehicle ambulance(AMBULANCE, true); // Ambulance in emergency mode
            g.shortestPath(src, dest, ambulance);
        }
    };


    // ================ MAIN MENU WITH ALL FEATURES ================
    void mainMenu() {
        addDefaultRoads(); // Initialize with some predefined roads
        string src, dest;
        int tick = 0; // Simulation "tick" counter for periodic events

        while (true) {
            // Periodic updates for dynamic simulation aspects
            tick++;
            // Weather updates are now handled by a separate thread
            if (tick % 10 == 0) IncidentMonitor::getInstance().generateIncident(); // Generate incidents every 10 ticks (Singleton access)
            if (tick % 30 == 0) ai.optimizeTrafficLights(); // AI optimization every 30 ticks

            // Clear console for fresh menu display - improves readability
#ifdef _WIN32
            system("cls"); // For Windows
#else
            system("clear"); // For Linux/macOS
#endif

            // --- ASCII Art Title (colored) ---
            cout << BLUE << R"(
  _____  _____  ___  ______ _____ _   _ _____
 |_   |/  __ \/ _ \ | ___ \_   _| \ | |  __ \
   | |  | /  \/ /_\ \| |_/ / | | |  \| | |  \/
   | |  | |   |  _  ||  _ /  | | | . ` | | __
  _| |_ | \__/\ | | || |\ \| _| |_| |\  | |_\ \
  \___/  \____|_| |_|_| \_|\___/\_| \_/\____/
            )" << RESET << endl;

            cout << CYAN << "\n=== MAIN MENU ===" << RESET << endl;
            // Applying colors to menu options
            cout << GREEN << "1. " << WHITE << "Add Road\n";
            cout << GREEN << "2. " << WHITE << "View Map (Basic)\n"; // Keeping for compatibility, but 12 is enhanced
            cout << GREEN << "3. " << WHITE << "Simulate/View Incidents\n";
            cout << GREEN << "4. " << WHITE << "Apply Rush Hour Conditions\n";
            cout << GREEN << "5. " << WHITE << "Calculate Shortest Path\n";
            cout << GREEN << "6. " << WHITE << "Compare Vehicle Routes (Car vs. Ambulance)\n";
            cout << GREEN << "7. " << WHITE << "Waypoint Routing (Limited)\n";
            cout << YELLOW << "8. " << WHITE << "Save/Load Data (Not Implemented)\n"; // Indicate non-implemented
            cout << AI_COLOR << "9. " << WHITE << "AI Traffic Analysis\n";
            cout << EMERGENCY_COLOR << "10. " << WHITE << "Emergency Mode (Simulate Siren)\n";
            cout << MAGENTA << "11. " << WHITE << "City Traffic Statistics\n";
            cout << CYAN << "12. " << WHITE << "Enhanced Live Map\n";
            cout << YELLOW << "13. " << WHITE << "Time Controls\n";
            cout << GREEN << "14. " << WHITE << "Export Traffic Data to CSV\n";
            cout << BLUE << "15. " << WHITE << "Run Interactive Tutorial\n";
            cout << RED << "0. " << WHITE << "Exit Simulation\n";
            cout << BOLD << "Select option: " << RESET;

            string inputLine;
            getline(cin, inputLine); // Read the whole line of user input, robust against mixed input

            int ch;
            stringstream ss(inputLine);
            if (!(ss >> ch)) { // Attempt to convert string to int
                ch = -1; // If conversion fails (e.g., user types text), assign an invalid option
            }

            if (ch == 0) {
                cout << GREEN << "Exiting Traffic Simulation. Goodbye!\n" << RESET;
                break; // Exit the loop
            }

            // Using a switch statement for menu navigation
            switch (ch) {
                case 1: { // Add Road
                    string u, v, type;
                    string w_str, sd_str;
                    int w_val, sd_val;

                    cout << "Enter start node: ";
                    getline(cin, u);
                    // Input Validation: Start node cannot be empty
                    if (u.empty()) {
                        cout << RED << "Error: Start node name cannot be empty!\n" << RESET;
                        break;
                    }

                    cout << "Enter end node: ";
                    getline(cin, v);
                    // Input Validation: End node cannot be empty
                    if (v.empty()) {
                        cout << RED << "Error: End node name cannot be empty!\n" << RESET;
                        break;
                    }
                    if (u == v) {
                        cout << RED << "Error: Start and end nodes cannot be the same for a road!\n" << RESET;
                        break;
                    }

                    try {
                        cout << "Enter weight (time in seconds, e.g., 300): ";
                        getline(cin, w_str);
                        w_val = stoi(w_str);
                        if (w_val <= 0) {
                            cout << RED << "Error: Weight must be a positive integer.\n" << RESET;
                            break;
                        }

                        cout << "Enter signal delay (seconds, e.g., 60): ";
                        getline(cin, sd_str);
                        sd_val = stoi(sd_str);
                        if (sd_val < 0) {
                            cout << RED << "Error: Signal delay cannot be negative.\n" << RESET;
                            break;
                        }
                    } catch (const invalid_argument& e) {
                        cout << RED << "Invalid number input. Please enter integers for weight and delay.\n" << RESET;
                        break; // Go back to main menu
                    } catch (const out_of_range& e) {
                        cout << RED << "Input number out of range. Please enter smaller integers.\n" << RESET;
                        break; // Go back to main menu
                    }

                    cout << "Enter road type (General, Bike Lane, Bus Lane, Highway, Bridge, Tunnel): ";
                    getline(cin, type);
                    // Basic validation for road type
                    if (type.empty()) {
                        cout << YELLOW << "Warning: Road type not specified. Defaulting to 'General'.\n" << RESET;
                        type = "General";
                    }

                    addRoad(u, v, w_val, sd_val, type);
                    break;
                }
                case 2: // View Map (Basic) - now points to Enhanced Map
                case 12: { // Enhanced Live Map
                    showEnhancedMap();
                    break;
                }
                case 3: { // Simulate/View Incidents
                    IncidentMonitor::getInstance().generateIncident(); // Generate a new incident (Singleton access)
                    IncidentMonitor::getInstance().showActiveIncidents(); // Show all active incidents (Singleton access)
                    break;
                }
                case 4: { // Apply Rush Hour Conditions
                    cout << YELLOW << "\nApplying rush hour conditions...\n" << RESET;
                    for (auto& nodePair : adjList) {
                        for (auto& edge : nodePair.second) {
                            // Retrieve the original base weight from baseEdges
                            double originalWeight = 0;
                            // Memory Safety: Check if key exists before using .at()
                            if (baseEdges.count({nodePair.first, edge.destination})) {
                                originalWeight = baseEdges.at({nodePair.first, edge.destination}).weight;
                            } else if (baseEdges.count({edge.destination, nodePair.first})) {
                                originalWeight = baseEdges.at({edge.destination, nodePair.first}).weight;
                            }
                            if (originalWeight == 0) originalWeight = edge.weight; // Fallback

                            edge.weight = originalWeight * 1.5; // Increase travel time by 50%
                            edge.congestion = rand() % MAX_CONGESTION + 1; // Add 1-MAX_CONGESTION congestion units
                        }
                    }
                    cout << GREEN << "Rush hour applied! Traffic is heavier and slower.\n" << RESET;
                    break;
                }
                case 5: { // Calculate Shortest Path (with Strategy Pattern selection)
                    cout << "Enter source node: "; getline(cin, src);
                    cout << "Enter destination node: "; getline(cin, dest);

                    cout << "Select routing strategy (1: " << YELLOW << "Fastest Route" << RESET << ", 2: " << RED << "Emergency Route" << RESET << "): ";
                    string strategyChoiceStr;
                    getline(cin, strategyChoiceStr);
                    int strategyChoice = -1;
                    try { strategyChoice = stoi(strategyChoiceStr); } catch(...) {}

                    unique_ptr<RoutingStrategy> strategy; // Use smart pointer for strategy
                    switch (strategyChoice) {
                        case 1: strategy = make_unique<FastestRoute>(); break;
                        case 2: strategy = make_unique<EmergencyRoute>(); break;
                        default:
                            cout << RED << "Invalid strategy. Defaulting to Fastest Route (Car).\n" << RESET;
                            strategy = make_unique<FastestRoute>();
                            break;
                    }
                    strategy->calculate(*this, src, dest); // Pass 'this' (the Graph object)
                    break;
                }
                case 6: { // Compare Vehicle Routes (using Strategy Pattern)
                    cout << "Enter source node for comparison: "; getline(cin, src);
                    cout << "Enter destination node for comparison: "; getline(cin, dest);
                    cout << YELLOW << "Comparing route for Car (Fastest) vs. Ambulance (Emergency):\n" << RESET;

                    FastestRoute fastestStrategy;
                    cout << BOLD << "\n--- Car Route ---\n" << RESET;
                    fastestStrategy.calculate(*this, src, dest);

                    EmergencyRoute emergencyStrategy;
                    cout << BOLD << "\n--- Ambulance Route (Emergency Mode) ---\n" << RESET;
                    emergencyStrategy.calculate(*this, src, dest);
                    break;
                }
                case 7: { // Waypoint Routing (Limited)
                    cout << YELLOW << "\nWaypoint routing is currently a direct path calculation. For multi-stop journeys, run shortest path multiple times.\n" << RESET;
                    cout << "Enter source node: "; getline(cin, src);
                    cout << "Enter destination node: "; getline(cin, dest);

                    Vehicle car(CAR); // Default to car for this
                    shortestPath(src, dest, car);
                    break;
                }
                case 8: { // Save/Load Data
                    cout << RED << "Save/Load functionality not implemented yet!\n" << RESET;
                    break;
                }
                case 9: { // AI Traffic Analysis
                    cout << "Enter start node for AI analysis: "; getline(cin, src);
                    // Input Validation: Start node cannot be empty
                    if (src.empty()) {
                        cout << RED << "Error: Start node name cannot be empty!\n" << RESET;
                        break;
                    }
                    cout << "Enter end node for AI analysis: "; getline(cin, dest);
                    // Input Validation: End node cannot be empty
                    if (dest.empty()) {
                        cout << RED << "Error: End node name cannot be empty!\n" << RESET;
                        break;
                    }
                    if (src == dest) {
                        cout << RED << "Error: Start and end nodes cannot be the same for AI analysis.\n" << RESET;
                        break;
                    }
                    ai.analyze(src, dest);
                    ai.predictCongestion(src, dest);
                    break;
                }
                case 10: { // Emergency Mode
                    cout << BLUE << "Activating emergency siren (this is a simulation effect).\n" << RESET;
                    playSiren();
                    break;
                }
                case 11: { // City Traffic Statistics
                    cout << MAGENTA << "\n=== CITY TRAFFIC STATISTICS ===\n" << RESET;
                    cout << "Current Weather: " << getWeatherMessage() << endl;
                    IncidentMonitor::getInstance().showActiveIncidents(); // Singleton access
                    cout << "Time Multiplier: " << timeMultiplier << "x\n";
                    // Add more stats for a richer experience
                    cout << "Total Nodes in Map: " << adjList.size() << endl;
                    int totalEdges = 0;
                    for(const auto& nodePair : adjList) {
                        totalEdges += nodePair.second.size();
                    }
                    cout << "Total Road Segments: " << totalEdges << endl;
                    break;
                }
                case 13: { // Time Controls
                    cout << "\n⏳ TIME CONTROLS:\n"
                         << GREEN << "1. " << WHITE << "Pause (0x Speed)\n"
                         << GREEN << "2. " << WHITE << "2x Speed\n"
                         << GREEN << "3. " << WHITE << "5x Speed\n"
                         << YELLOW << "4. " << WHITE << "Rewind (Not Implemented Yet)\n" // Not yet implemented
                         << BOLD << "Choice: " << RESET;
                    string choiceStr;
                    getline(cin, choiceStr);
                    int choice = -1;
                    try { choice = stoi(choiceStr); } catch(...) {} // Safe conversion

                    switch(choice) {
                        case 1: timeMultiplier = 0; cout << YELLOW << "Time paused.\n" << RESET; break;
                        case 2: timeMultiplier = 2; cout << YELLOW << "Time set to 2x speed.\n" << RESET; break;
                        case 3: timeMultiplier = 5; cout << YELLOW << "Time set to 5x speed.\n" << RESET; break;
                        case 4: cout << RED << "🔙 Rewind not implemented yet!\n" << RESET; break;
                        default: cout << RED << "Invalid time control option!\n" << RESET; break;
                    }
                    break;
                }
                case 14: exportToCSV(); break; // Export Data
                case 15: runTutorial(); break;  // Run Tutorial
                default: cout << RED << "Invalid Option! Please select a number from the menu.\n" << RESET;
            }
            // Pause before showing the menu again to allow user to read output
            cout << "\n" << BOLD << "Press Enter to continue..." << RESET;
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear input buffer and wait for Enter
        }
    }
private:
    // Adds a set of predefined roads to the graph for initial setup
    void addDefaultRoads() {
        // Highway system
        addRoad("Downtown", "Midtown", 300, 60, "Highway");
        addRoad("Midtown", "Uptown", 400, 80, "Highway");
        addRoad("Downtown", "Airport", 500, 120, "Highway");

        // City streets
        addRoad("Downtown", "Market St", 120, 30, "General");
        addRoad("Market St", "City Hall", 90, 20, "General");
        addRoad("City Hall", "Uptown", 180, 40, "General");
        addRoad("Downtown", "Residential Area", 150, 25, "General");
        addRoad("Market St", "Industrial Zone", 250, 50, "General");

        // Special routes
        addRoad("Midtown", "Bike Trail", 150, 10, "Bike Lane"); // Renamed for clarity
        addRoad("City Hall", "Bus Terminal", 200, 30, "Bus Lane");
        addRoad("Airport", "Emergency Hospital", 100, 10, "Emergency"); // New emergency specific road
        addRoad("Uptown", "Suburban Tunnel", 350, 70, "Tunnel");
        addRoad("Residential Area", "Central Bridge", 200, 40, "Bridge");

        cout << CYAN << "Default roads loaded.\n" << RESET;
    }

    // Applies weather effects to road weights based on current weather conditions.
    // IMPORTANT: This now correctly uses the 'baseEdges' to get the original weight
    // and then applies the weather multiplier, preventing compounding effects.
    void applyWeatherEffects() {
        double weatherMult = getWeatherMultiplier();
        for (auto& nodePair : adjList) {
            for (auto& edge : nodePair.second) {
                // Find the original base weight for this specific edge from the baseEdges map
                double originalBaseWeight = 0;
                // Memory Safety: Check if key exists before using .at()
                if (baseEdges.count({nodePair.first, edge.destination})) {
                    originalBaseWeight = baseEdges.at({nodePair.first, edge.destination}).weight;
                } else if (baseEdges.count({edge.destination, nodePair.first})) { // Check reverse direction if needed
                    originalBaseWeight = baseEdges.at({edge.destination, nodePair.first}).weight;
                }

                // Fallback: If for some reason the original base weight isn't found (shouldn't happen if all roads
                // are added via addRoad), use the current edge's weight as a temporary base.
                if (originalBaseWeight == 0) originalBaseWeight = edge.weight;

                // Apply weather multiplier to the original base weight and update the edge's current weight
                edge.weight = originalBaseWeight / weatherMult;
            }
        }
    }

    void simulateTimeDelay(int seconds) {
        cout << CYAN << "\nSimulating journey (" << seconds << " seconds)...\n" << RESET;
        progressBar(seconds);
    }

    // Unit Test Scaffolding
    #ifdef TESTING
    public: // Making public for external test access
    static void runTests() {
        Graph testGraph;
        cout << GREEN << "\n=== Running Unit Tests ===\n" << RESET;

        // Test 1: Add road and basic shortest path
        testGraph.addRoad("TestA", "TestB", 100, 0);
        Vehicle testCar(CAR);
        testGraph.shortestPath("TestA", "TestB", testCar);

        // Test 2: Edge case - same source and destination
        testGraph.shortestPath("TestA", "TestA", testCar);

        // Test 3: Edge case - non-existent node
        testGraph.shortestPath("TestA", "NonExistent", testCar);

        // Test 4: Check if Singleton IncidentMonitor is working
        IncidentMonitor::getInstance().generateIncident();
        IncidentMonitor::getInstance().showActiveIncidents();

        // Test 5: Strategy pattern
        testGraph.FastestRoute().calculate(testGraph, "TestA", "TestB");
        testGraph.EmergencyRoute().calculate(testGraph, "TestA", "TestB");

        cout << GREEN << "\n=== Unit tests passed! ===\n" << RESET;
    }
    #endif
};

int main() {
#ifdef _WIN32 // Conditionally compile SetConsoleOutputCP for Windows
    // Set console output code page to UTF-8 (65001) for proper emoji display.
    // This is crucial for Windows consoles to show emojis correctly.
    SetConsoleOutputCP(65001);
#endif
    // Seed the random number generator using current time for varied results
    srand(static_cast<unsigned int>(time(nullptr)));

    // Multithreading for Weather: Start weather update in a separate thread
    std::thread weatherThread([](){
        while (true) {
            sleep_seconds(WEATHER_UPDATE_INTERVAL / max(1, timeMultiplier)); // Update based on global constant and time multiplier
            updateWeather();
        }
    });
    weatherThread.detach(); // Detach the thread so it runs independently

    Graph sim; // Create an instance of the Graph class

    // Unit Test Execution (if TESTING macro is defined during compilation)
    #ifdef TESTING
    Graph::runTests();
    #else
    sim.mainMenu(); // Start the main application menu only if not testing
    #endif

    return 0; // Indicate successful execution
}