#include <iostream>
#include<climits>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <utility>
#include <sys/stat.h>  // For mkdir function
#include <filesystem>
#include <iomanip>

using namespace std;

namespace fs = filesystem;

class CompleteGraph {
public:
    CompleteGraph(const vector<pair<string, string>>& pairs) : pairs(pairs) {}

    unordered_map<string, set<string>> construct() {
        unordered_map<string, set<string>> graph;
        for (const auto& pair1 : pairs) {
            for (const auto& pair2 : pairs) {
                if (pair1 != pair2) {
                    graph[pair1.first + " " + pair1.second].insert(pair2.first + " " + pair2.second);
                }
            }
        }
        return graph;
    }

private:
    vector<pair<string, string>> pairs;
};

class GraphProcessor {
public:
    friend class TimeTable_Generator;
    const map<int, set<string>>& getColorNodeMap() const {
        return colorNodeMap;
    }
    void mergeGraphs(const map<int, vector<pair<string, string>>>& graphData) {
        for (const auto& kv : graphData) {
            CompleteGraph completeGraph(kv.second);
            const auto& graph = completeGraph.construct();
            merge(graph);
        }
        colorGraph();
    }

    void printColorMap() const {
        cout << "Color Node Map:\n";
        for (const auto& entry : colorNodeMap) {
            cout << "Color " << entry.first << ": ";
            for (const auto& node : entry.second) {
                cout << node << " ";
            }
            cout << "\n";
        }

        cout << "\nColor Usage:\n";
        for (const auto& entry : colorUsage) {
            cout << "Color " << entry.first << " used " << entry.second << " times\n";
        }
    }

private:
    unordered_map<string, set<string>> graphData;
    unordered_map<string, int> colorMap;
    map<int, set<string>> colorNodeMap;  //stores the colors to subjects mapping
    unordered_map<int, int> colorUsage; // Added to track color usage
    int total_colors = 0;

    void merge(const unordered_map<string, set<string>>& newGraph) {
        for (const auto& node : newGraph) {
            graphData[node.first].insert(node.second.begin(), node.second.end());
        }
    }

    void colorGraph() {
        vector<pair<string, int>> degrees;
        for (const auto& node : graphData) {
            int degree = node.second.size();
            degrees.push_back({node.first, degree});
        }

        sort(degrees.begin(), degrees.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        unordered_set<int> usedColors;

        for (const auto& entry : degrees) {
            const auto& node = entry.first;
            int color = 1;

            for (const auto& neighbor : graphData[node]) {
                if (colorMap.find(neighbor) != colorMap.end()) {
                    usedColors.insert(colorMap[neighbor]);
                }
            }

            int minused = INT_MAX;
            int bcol=-1;

            for(const auto &col: colorUsage)
            {
                if(usedColors.find(col.first) == usedColors.end())
                {
                    if(col.second < minused)
                    {
                        minused = min(minused, col.second);
                        bcol = col.first;
                    }
                    
                }

            }

            if(bcol != -1)
            {
                color = bcol;
            }
            else
            {
                while (usedColors.find(color) != usedColors.end()) 
                {
                    color++;
                }
            }

            colorMap[node] = color;
            colorNodeMap[color].insert(node);

            colorUsage[color]++; // Increment the usage count for the color

            usedColors.clear();
        }
    }
};

class TeacherSubjects {
public:
    TeacherSubjects(const string& filename) : filename(filename) {}
    const vector<string>& getSections() const {
        return sections;
    }
    void processFile() {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Failed to open the file." << endl;
            return;
        }

        sections = readSections(file);
        teacherSubjects = readData(file, sections);

        file.close();
    }

    const map<int, vector<pair<string, string>>>& getTeacherSubjectsMap() const {
        return teacherSubjects;
    }


    void updateFile(const string& updateFilename) {
        ifstream updateFile(updateFilename);
        if (!updateFile.is_open()) {
            cerr << "Failed to open the update file." << endl;
            return;
        }

        string line;
        int lineNumber = 0;
        set<int> delteacher;

        while (getline(updateFile, line)) {
            lineNumber++;

            // Skip the header line (lineNumber == 1)
            if (lineNumber == 1) {
                continue;
            }

            istringstream iss(line);

            string oldTeacherIdStr, section, newTeacherIdStr;
            if (getline(iss, oldTeacherIdStr, ',') && getline(iss, section, ',') && getline(iss, newTeacherIdStr, ',')) {
                try {
                    int oldTeacherId = stoi(oldTeacherIdStr);
                    int newTeacherId = stoi(newTeacherIdStr);

                    // Check if the old teacher ID exists in the map
                    if (teacherSubjects.find(oldTeacherId) != teacherSubjects.end()) {
                        // Retrieve the subjects for the old teacher
                        vector<pair<string, string>> oldSubjects = teacherSubjects[oldTeacherId];

                        // Concatenate subjects of old teacher to the new teacher for the specified section
                        for (auto& subject : oldSubjects) {
                            if (subject.first == section) {
                                teacherSubjects[newTeacherId].push_back({section, subject.second});
                            }
                        }

                        // Erase the old teacher from the map
                        delteacher.insert(oldTeacherId);
                        

                    } else {
                        cerr << "Warning: Old teacher ID " << oldTeacherId << " not found in the map." << endl;
                    }

                } catch (const invalid_argument& e) {
                    cerr << "Error converting to integer: " << e.what() << endl;
                    // Handle the error, possibly by skipping the line or taking other actions
                }
            }
        }

        for(auto oteacher: delteacher)
        {
            teacherSubjects.erase(oteacher);
        }

        updateFile.close();
    }

    void printResults() const {
        for (const auto &teacher : teacherSubjects) {
            cout << "Teacher " << teacher.first << " teaches subjects: ";
            for (const auto &subject : teacher.second) {
                cout << subject.first << "-" << subject.second << " ";
            }
            cout << endl;
        }
    }

private:
    string filename;
    vector<string> sections;
    map<int, vector<pair<string, string>>> teacherSubjects;

    vector<string> readSections(ifstream& file) {
        vector<string> sections;
        string line;
        getline(file, line);

        string field;
        istringstream iss(line);

        if (getline(iss, field, ',')) {
            while (getline(iss, field, ',')) {
                sections.push_back(field);
            }
        }

        for (auto &sec : sections) {
            cout << sec << " ";
        }
        cout << endl;

        return sections;
    }

    map<int, vector<pair<string, string>>> readData(ifstream& file, const vector<string>& sections) {
        map<int, vector<pair<string, string>>> teacherSubjects;
        string line;
        int i = 0;

        while (getline(file, line)) {
            istringstream iss(line);

            string field;
            if (getline(iss, field, ',')) {
                try {
                    int teacherId = stoi(field);

                    while (getline(iss, field, ',')) {
                        if (!field.empty())
                            teacherSubjects[teacherId].push_back({sections[i], field});

                        ++i;
                    }
                } catch (const invalid_argument& e) {
                    cerr << "Error converting to integer: " << e.what() << endl;
                    // Handle the error, possibly by skipping the line or taking other actions
                }
            }
            i = 0;
        }

        return teacherSubjects;
    }
};

class TimeTable_Generator {
private:
    int tot_slots = 15;
    map<string,vector<pair<int,string>>>slots; //string - section vector-slots(slotid subname)


public:

        void populateSlotsManually(vector<string> sections) 
        {
        // Sample data
            // vector<string> sections = {"A SEC", "B SEC", "C SEC", "D SEC"};
            vector<string> subjects = {"Math", "English", "History", "Chemistry", "Physics"};

            // Manually populate slots for each section
            for (const auto& sec : sections) {
                vector<pair<int, string>> sectionSlots;

                for (int i = 1; i <= tot_slots; ++i) {
                    // For testing, let's assign subjects in a cyclic manner
                    string subject = subjects[(i - 1) % subjects.size()];
                    sectionSlots.push_back({i, subject});
                }

                slots[sec] = sectionSlots;
            }
        }


    void writeTimetableToCSV(const vector<string>& sections) const {
        vector<string> days = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
        int slotsPerDay = tot_slots / days.size();
        

        #ifdef _WIN32
            mkdir("timetables");
        #else
            mkdir("timetables", 0777);
        #endif
        string folderPath = "timetables/";

        for (const auto& sec : sections) {
            vector<pair<int, string>> sectionSlots = slots.at(sec);
            
            string filename = folderPath + sec + "_timetable.csv";
            ofstream outputFile(filename);

            if (outputFile.is_open()) {
                outputFile << "Day";
                for (int i = 1; i <= slotsPerDay; ++i) {
                    outputFile << ",Slot " << i;
                }
                outputFile << "\n";

                int used_slots = 0;

                for (const auto& day : days) {
                    outputFile << day;

                    int slotsAssigned = 0;

                    for (; slotsAssigned < slotsPerDay; ++slotsAssigned) {
                        // Write the subject name to the CSV file
                        outputFile << "," << sectionSlots[used_slots].second;
                        used_slots++;
                    }

                    // Fill any remaining slots with "Free"
                    for (int i = slotsAssigned; i < slotsPerDay; ++i) {
                        outputFile << ",Free";
                    }

                    outputFile << "\n";
                }

                outputFile.close();
                cout << "Timetable for section " << sec << " written to " << filename << endl;
            } else {
                cerr << "Error opening file for section " << sec << endl;
            }
        }
    }


    void print_table(const vector<string> sections)
    {
        for(const auto &sec: sections)
        {
            cout<<"Section "<<sec<<": ";
            for(const auto &slot: slots[sec])
            {
                cout<<slot.first<<"-"<<slot.second<<" ";
            }
            cout<<endl;
        }
    }


    void generateTimetable(const GraphProcessor& graphProcessor,const vector<string> sections) {
        
        const map<int, set<string>>& colorNodeMap = graphProcessor.getColorNodeMap();

        // Initialize slots
        for (const auto& sec : sections) {
            for (int i = 1; i <= tot_slots; i++) {
                slots[sec].push_back({i, "free"});
            }
        }

        int currentSlot = 1;

        for (const auto& entry : colorNodeMap) {
            int color = entry.first;
            const set<string>& nodes = entry.second;

            for (const auto& node : nodes) {
                // Find the section and subject from the node
                size_t pos = node.find(" ");
                string section = node.substr(0, pos);
                string subject = node.substr(pos + 1);
                cout<<section<<"-"<<subject<<" ";
                cout<<endl;

                // Try to find the next available slot for the section
                int attempts = 0;
                while (attempts < tot_slots && slots[section][currentSlot - 1].second != "free") {
                    currentSlot = (currentSlot % tot_slots) + 1;
                    attempts++;
                }

                // Check if all slots are occupied
                if (attempts == tot_slots && slots[section][currentSlot - 1].second != "free") {
                    cerr << "Error: All slots in section " << section << " are occupied." << endl;
                    return;
                }

                // Assign the subject to the current slot for the corresponding section
                slots[section][currentSlot - 1] = {currentSlot, subject};

                // Move to the next slot
                currentSlot = (currentSlot % tot_slots) + 1;
            }
        }
                    // print slots
            // print_table(sections);
        for(const auto &sec: sections)
        {
            cout<<"Section "<<sec<<": ";
            for(const auto &slot: slots[sec])
            {
                cout<<slot.first<<"-"<<slot.second<<" ";
            }
            cout<<endl;
        }
            
    } 
        
};

class User {
public:
     string username;
     string password;
     string role;

    User(const  string& u, const  string& p, const  string& r) : username(u), password(p), role(r) {}

    bool authenticate(const  string& enteredUsername, const  string& enteredPassword) const {
        return (enteredUsername == username) && (enteredPassword == password);
    }
};

class auth{
private:
      
    User* currentUser;
    vector<User> users;
    vector<string> fileNames;

public:
    static bool isLoggedIn;// Static member variable to track login status
    auth() : currentUser(nullptr) { 
        // Load user data from the CSV file
        users = readUserDataFromCSV("user.csv");
    }

    void viewtablenames()
    {
        string folderPath = "timetables/";
        

        try {
            for (const auto& entry : fs::directory_iterator(folderPath)) {
                if (entry.is_regular_file()) {
                    fileNames.push_back(entry.path().filename().string());
                }
            }

            // Print the file names
            int i=1;
            for (const auto& fileName : fileNames) {
                cout<<i<<". ";
                cout << fileName << endl;
                ++i;
            }
        } catch (const errc) {
            cerr << "Error accessing folder: " << endl;
        }
    }

    void approvetable() {
        int index;
        vector<int> approvedIndices;
        do {
            cout << "Enter the index of the table you want to approve (-1 to exit): ";
            cin >> index;

            if (index == -1) {
                cout << "Exiting approval process." << endl;
                
            }

            if (index > 0 && index <= fileNames.size()) {
                cout << "You have approved table: " << fileNames[index - 1] << endl;

                // Store the approved index
                approvedIndices.push_back(index);

                // Add your processing logic here for the approved table
            } else {
                cerr << "Invalid index. Please enter a valid index." << endl;
            }

        } while (index >= 0);
        // Create the "approved" folder if it doesn't exist
        #ifdef _WIN32
            mkdir("approved");
        #else
            mkdir("approved", 0777);
        #endif

        // Move approved files to the "approved" folder
        for (const auto approved : approvedIndices) {
            string sourceFile = "timetables/" + fileNames[approved - 1];
            string destinationFile = "approved/" + fileNames[approved - 1];

            try {
                fs::copy(sourceFile, destinationFile, fs::copy_options::overwrite_existing);
                fs::remove(sourceFile);

                cout << "Moved " << fileNames[approved - 1] << " to the 'approved' folder." << endl;
            } catch (const errc) {
                cerr << "Error moving file: "<< endl;
            }
        }
    }

    void displaytable(const string& path) {
        string folderPath = "./" + path;
        try {
                vector<string> tables;

                for (const auto& entry : fs::directory_iterator(folderPath)) {
                    if (entry.is_regular_file()) {
                        tables.push_back(entry.path().filename().string());
                    }
                }

                // Print the file names
                int i = 1;
                for (const auto& fileName : tables) {
                    cout << i << ". " << fileName << endl;
                    ++i;
                }

                int enter;
                cout << "Enter the table you want to view: ";
                cin >> enter;

                if (enter > 0 && enter <= tables.size()) {
                    string file = folderPath + "/" + tables[enter - 1];
                    ifstream inputFile(file);

                    if (!inputFile.is_open()) {
                        cerr << "Error opening file: " << file << endl;
                        return;
                    }

                    // Read the contents into a vector
                    vector<vector<string>> tableContents;
                    string line;
                    while (getline(inputFile, line)) {
                        vector<string> row;
                        size_t pos = 0;
                        while ((pos = line.find(',')) != string::npos) {
                            row.push_back(line.substr(0, pos));
                            line.erase(0, pos + 1);
                        }
                        row.push_back(line); // Add the last element
                        tableContents.push_back(row);
                    }

                    inputFile.close();

                    // Display the contents in a formatted way
                    cout << "Displaying contents of table: " << tables[enter - 1] << endl;
                    displayFormattedTable(tableContents);
                } else {
                    cerr << "Invalid table number. Please enter a valid number." << endl;
                }
            
            } catch (const errc) 
            {
            cerr << "Error accessing folder: " << endl;
            }
    }
    void displayFormattedTable(const vector<vector<string>>& table) {
        const int colWidth = 15;

        // Display headers
        for (const auto& header : table[0]) {
            cout << setw(colWidth) << left << header;
        }
        cout << endl;

        // Display data
        for (size_t i = 1; i < table.size(); ++i) {
            for (const auto& cell : table[i]) {
                cout << setw(colWidth) << left << cell;
            }
            cout << endl;
        }
    }


    void displayauth() {
        if (isLoggedIn) {
             cout << "User already logged in. Logout first.\n";
            return;
        }

         cout << "===== Login Page =====\n";
         string enteredUsername, enteredPassword;

         cout << "Enter username: ";
         cin >> enteredUsername;

         cout << "Enter password: ";
         cin >> enteredPassword;

        // Attempt to find the user in the vector
        auto userIt =  find_if(users.begin(), users.end(), [&](const User& user) {
            return user.authenticate(enteredUsername, enteredPassword);
        });

        if (userIt != users.end()) {
             cout << "Login successful! Role: " << userIt->role << "\n";
            currentUser = new User(userIt->username, userIt->password, userIt->role);
            isLoggedIn = true;
        } else {
             cout << "Login failed. Invalid username or password.\n";
        }
    }
    string get_currentuser_role()
    {
        return currentUser->role;
    }
    void displayWelcomeMessage() const {
        if (isLoggedIn) {
             cout << "Welcome, " << currentUser->username << "! (Role: " << currentUser->role << ")\n";
        } else {
             cout << "Please log in first.\n";
        }
    }

    void logout() {
        if (isLoggedIn) {
            delete currentUser;
            currentUser = nullptr;
            isLoggedIn = false;
             cout << "Logout successful!\n";
        } else {
             cout << "No user is currently logged in.\n";
        }
    }

    ~auth() {
        delete currentUser;
    }

private:
     vector<User> readUserDataFromCSV(const  string& filename) {
         vector<User> loadedUsers;

         ifstream inputFile(filename);
        if (!inputFile.is_open()) {
             cerr << "Error opening file: " << filename <<  endl;
            return loadedUsers;
        }

         string line;
        while ( getline(inputFile, line)) {
             istringstream iss(line);
             string username, password, role;

            // Assuming the CSV format is: username,password,role
            if ( getline(iss, username, ',') &&  getline(iss, password, ',') &&  getline(iss, role)) {
                loadedUsers.emplace_back(username, password, role);
            } else {
                 cerr << "Error parsing line: " << line <<  endl;
            }
        }

        inputFile.close();
        return loadedUsers;
    }
};


// Initialize the static member variable
bool auth::isLoggedIn = false;

int main() {
    // Create instances of classes
    auth auth;
    GraphProcessor graphProcessor;
    TeacherSubjects teacherSubjects("teacher_data.csv");
    TimeTable_Generator timeTableGenerator;

    // Display login page
    auth.displayauth();

    if(auth::isLoggedIn) {

        string role = auth.get_currentuser_role();

        while (true) {
            cout << "========== TimeTable Generator Menu ==========\n";

            if (role == "hod") {
                cout << "1. View Timetable\n2. Approve Timetable\n3. View Report\n4. Logout\n";
            } else if (role == "admin") {
                cout << "1. Generate Timetable\n2. Update Timetable\n3. View Report\n4. Logout\n";
            } else {
                cout << "1. View Timetable\n2. Logout\n";
            }

            cout << "Enter your choice (1-4): ";
            int choice;
            cin >> choice;
            const auto& sections = teacherSubjects.getSections();
            
            switch (choice) {
                case 1:
                    // View Timetable or Generate Timetable based on the role
                    if (role == "hod" || role == "teacher") {
                        // Display timetable
                        timeTableGenerator.print_table(sections);
                    } else if (role == "admin") {
                        // Generate timetable
                        graphProcessor.mergeGraphs(teacherSubjects.getTeacherSubjectsMap());
                        timeTableGenerator.generateTimetable(graphProcessor, sections);
                        timeTableGenerator.writeTimetableToCSV(sections);
                    }
                    break;

                case 2:
                    // Approve Timetable or Update Timetable based on the role
                    if (role == "hod") {
                        auth.approvetable();
                    } else if (role == "admin") {
                        teacherSubjects.updateFile("update.csv");
                        teacherSubjects.printResults();
                    }
                    break;

                case 3:
                    // View Report
                    graphProcessor.printColorMap();
                    break;

                case 4:
                    // Logout
                    auth.logout();
                    cout << "Exiting TimeTable Generator. Goodbye!\n";
                    return 0;

                default:
                    cout << "Invalid choice. Please enter a valid option.\n";
            }
        }
    } else {
        cout << "Login failed. Exiting TimeTable Generator.\n";
    }

    return 0;
}
