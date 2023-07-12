// #include "../include/interface.h"

// Interface::Interface(
//     bool is_participant,
//     shared_ptr<WorkStationTable> participants)
// {
//     this->is_participant = is_participant;
//     this->participants = participants;
// };

// //Get terminal input and validate it according to station (participant or manager)
// void Interface::terminal()
// {
//     bool valid_input = false;
    
//     while (valid_input == false)
//     {
//         vector<string> input_vec;
//         string terminal_input;

//         getline(cin,terminal_input);
//         transform(terminal_input.begin(), terminal_input.end(), terminal_input.begin(), ::toupper);

//         int space_position = terminal_input.find(" ");
//         string cmd = terminal_input.substr(0, space_position);
//         string cmd_argument = terminal_input.substr(space_position+1);

//         if (this->is_participant)
//         {
//             if (cmd == EXIT)
//                 valid_input = true;
//             else
//                 cout << "The only valid command for participant station is 'EXIT'" << endl;
//         }
//         else
//         {
//             if (cmd == WAKEUP && cmd_argument != cmd) //if no hostname is informed, no argument will have been found in the terminal command and substrings will be equal
//             {
//                 string hostname = cmd_argument;

//                 //wakeup(hostname,&this->participants); //Not a real method, just writing out program flow
//                 show_participants(); //Show updated participants list

//                 valid_input = true;
//             }
//             else
//                 cout << "The only valid command for manager station is 'WAKEUP <hostname>'" << endl;
//         };
//     };

// };

// void Interface::show_participants()
// {
//     /*
//     * Show participants
//     */
// };

// void Interface::show_manager()
// {
//     /*
//     * Show manager station
//     */
// };
