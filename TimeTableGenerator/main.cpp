#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

int main() {
    ifstream file("data.csv");
    if (!file.is_open()) {
        cerr << "Failed to open the file." << endl;
        return 1;
    }

    map<int, vector<pair<string,string>>> teacher_subjects;

    vector<string> sections;

    string line;
    getline(file, line);

    string field;
    istringstream iss(line);

    if (getline(iss, field, ','))
    {
        while (getline(iss, field, ',')) 
        {
            sections.push_back(field);
        }
    }
    
    for(auto &sec:sections)
    {
        cout<<sec<<" ";
    }
    cout<<endl;

    int i = 0;
    while (getline(file, line)) {

        istringstream iss(line);

        if (getline(iss, field, ',')) {
            try {
                int teacher_id = stoi(field);

                while (getline(iss, field, ',')) {
                    if (!field.empty())
                        teacher_subjects[teacher_id].push_back({sections[i],field});

                    ++i;
                }
            } catch (const invalid_argument& e) {
                cerr << "Error converting to integer: " << e.what() << endl;
                // Handle the error, possibly by skipping the line or taking other actions
            }
        }
        i=0;
    }

    for(auto &a:teacher_subjects)
    {
        cout << "Teacher " << a.first << " teaches subjects: ";
        for(auto &sub :a.second)
        {
            cout<<sub.first<<"-"<<sub.second<<" ";
        }
        cout<<endl;
    }


    file.close();
    return 0;
}
