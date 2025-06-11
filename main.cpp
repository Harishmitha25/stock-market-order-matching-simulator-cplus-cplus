#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

//Struct for the Order type
struct Order {
    string orderId;
    char orderType;
    int quantity;
    double limitPrice;
    int arrivalTime;
};

//Comparator to prioritise the buy and sell order respectively based on the rules
struct OrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.limitPrice == -1 && b.limitPrice != -1) return true;
        if (b.limitPrice == -1 && a.limitPrice != -1) return false;

        if (a.orderType == 'B' && b.orderType == 'B') {
            if (a.limitPrice != b.limitPrice) return a.limitPrice > b.limitPrice;
        }

        if (a.orderType == 'S' && b.orderType == 'S') {
            if (a.limitPrice != b.limitPrice) return a.limitPrice < b.limitPrice;
        }

        return a.arrivalTime < b.arrivalTime;
    }
};

//Separate multiset for buy and sell orders to automatically prioritise and sort the orders based on the rules and vector to track unexecuted orders
multiset<Order, OrderComparator> buyOrders;
multiset<Order, OrderComparator> sellOrders;
vector<Order> allOrders;
double latestPrice = 0.0;

//Update order quantity in the orders in the vector to display the unexecuted orders 
void updateOrderQuantity(const string& orderId, int remainingQuantity) {
    for (auto& order : allOrders) {
        if (order.orderId == orderId) {
            order.quantity = remainingQuantity;
            break;
        }
    }
}

//Display the pendings orders
void displayPendingOrders() {
    cout << "Latest price: " <<  latestPrice << endl;
    cout << "Buy Orders: " << endl;
    for (const auto& order : buyOrders) {
        if (order.limitPrice == -1) {
        cout << order.orderId << " M " << order.quantity << endl;
        } else {
            cout << order.orderId << " " << order.limitPrice << " " << order.quantity << endl;
        }
    }
    cout<<endl;
    cout << "Sell Orders: " << endl;
    for (const auto& order : sellOrders) {
        if (order.limitPrice == -1) {
            cout << order.orderId << " M " << order.quantity << endl;
        } else {
            cout << order.orderId << " " <<order.limitPrice << " " << order.quantity << endl;
        }
    }
        cout<<endl;

    cout << "---------------------------------------------------" << endl;
    cout << endl;
}

//Process orders based on the type of order read
void processOrder(Order& newOrder, ofstream& outputFile) {
    if (newOrder.orderType == 'B') { // Buy order
        auto it = sellOrders.begin();
        while (it != sellOrders.end() && newOrder.quantity > 0) { //Run the loop until end of sell orders
        //is reached anfd the new order's (buy order) limit price is greater than the current sell order's limit price
            if (newOrder.limitPrice != -1 && it->limitPrice != -1 && newOrder.limitPrice < it->limitPrice) { //If either is a
            // market order and buy order limit price less than current sell order limit price then break
                break;
            }

            // Quantity - minimum of the two because only that amount of shares can be executed
            int execQuantity = min(newOrder.quantity, it->quantity);
            double execPrice;
            
            //If buy order is market order and current sell order is not market order
            //then the execPrice is the sell order's limit price else last transaction price, vice vera for
            //if the current sell order is market order and if both are limit orders then current sell order's limit price
            if (newOrder.limitPrice == -1) {
                execPrice = (it->limitPrice != -1) ? it->limitPrice : latestPrice;
            } else if (it->limitPrice == -1) {
                execPrice = (newOrder.limitPrice != -1) ? newOrder.limitPrice : latestPrice;
            } else {
                execPrice = it->limitPrice;
            }

            //Write to file
            outputFile << "order " << newOrder.orderId << " " << execQuantity << " shares purchased at price "
                       << fixed << setprecision(2) << execPrice << endl;
            outputFile << "order " << it->orderId << " " << execQuantity << " shares sold at price "
                       << fixed << setprecision(2) << execPrice << endl;

            latestPrice = execPrice;
            newOrder.quantity -= execQuantity;
            updateOrderQuantity(newOrder.orderId, newOrder.quantity);

            const_cast<Order&>(*it).quantity -= execQuantity;
            updateOrderQuantity(it->orderId, it->quantity);

            //If current sell order is executed fully then erase it from the sell orders multiset else move the iterato 
            if (it->quantity == 0) {
                it = sellOrders.erase(it);
            } else {
                it++;
            }
        }

        //If new order (buy) quantity is greater than 0 (partially executed) then add it to the but orders multiset
        if (newOrder.quantity > 0) {
            buyOrders.insert(newOrder);
        }
    } else { // sell order
        auto it = buyOrders.begin();
        while (it != buyOrders.end() && newOrder.quantity > 0) { //Run the loop until end of buy orders
        //is reached anfd the new order's (sell order) limit price is greater than the current buy order's limit price
            if (newOrder.limitPrice != -1 && it->limitPrice != -1 && newOrder.limitPrice > it->limitPrice) //If either is a
            // market order and sell order limit price greater than current buy order limit price then break
                break;

            // Quantity - minimum of the two because only that amount of shares can be executed
            int execQuantity = min(newOrder.quantity, it->quantity);
            double execPrice;

            //If sell order is market order and current buy order is not market order
            //then the execPrice is the buy order's limit price else last transaction price, vice vera for
            //if the current buy order is market order and if both are limit orders then current biy order's limit price
            if (newOrder.limitPrice == -1) {
                execPrice = (it->limitPrice != -1) ? it->limitPrice : latestPrice;
            } else if (it->limitPrice == -1) {
                execPrice = (newOrder.limitPrice != -1) ? newOrder.limitPrice : latestPrice;
            } else {
                execPrice = it->limitPrice;
            }
            //Write to file
            outputFile << "order " << it->orderId << " " << execQuantity << " shares purchased at price "
                       <<execPrice << endl;
            outputFile << "order " << newOrder.orderId << " " << execQuantity << " shares sold at price "
                       <<execPrice << endl;

            latestPrice = execPrice;
            newOrder.quantity -= execQuantity;
            updateOrderQuantity(newOrder.orderId, newOrder.quantity);

            const_cast<Order&>(*it).quantity -= execQuantity;
            updateOrderQuantity(it->orderId, it->quantity);

            //If current buy order is executed fully then erase it from the sell orders multiset else move the iterato 
            if (it->quantity == 0) {
                it = buyOrders.erase(it);
            } else {
                it++;
            }
        }

        //If new order (sell) quantity is greater than 0 (partially executed) then add it to the but orders multiset
        if (newOrder.quantity > 0) {
            sellOrders.insert(newOrder);
        }
    }

    displayPendingOrders();
}


int main(int argc, char* argv[]) {
    cout << fixed << setprecision(2);

    string inputFileName = argv[1];
    ifstream inputFile(inputFileName);

    // Create the output file name based on input file name
    int start = inputFileName.find_first_of("0123456789");
    int end = inputFileName.find(".txt");
    string number;

    if (start != -1 && end != -1) {
        number = inputFileName.substr(start, end - start);
    }

    string outputFileName = "output" + number+".txt";
    ofstream outputFile(outputFileName);
    
    outputFile <<  fixed << setprecision(2);

    string line;
    int arrivalTime = 0;

    if (getline(inputFile, line)) {
        latestPrice = stod(line);
    }

    //Read each line in the input file
    while (getline(inputFile, line)) {
        istringstream lineStream(line);
        string orderId, type;
        int quantity;
        double limitPrice = -1;

        lineStream >> orderId >> type >> quantity;
        if (lineStream >> limitPrice) {
        }

        //Create and initialise fields of Order struct
        Order newOrder;
        newOrder.orderId = orderId;
        newOrder.orderType = type[0];
        newOrder.quantity = quantity;
        newOrder.limitPrice = limitPrice;
        newOrder.arrivalTime++;
        allOrders.push_back(newOrder); //Add the orders to the vector
        processOrder(newOrder, outputFile);
    }

    int unexecutedCount = 0;
    for (const auto& order : allOrders) {
        if (order.quantity > 0) {
            unexecutedCount++;
        }
    }
    
    //Display unexecuted orders one by one according to their arrival time
    int count = 0;
    for (const auto& order : allOrders) {
        if (order.quantity > 0) {
            outputFile << "order " << order.orderId << " " << order.quantity
                       << " shares unexecuted";
            count++;
            if (count < unexecutedCount) {
                outputFile << endl;
            }
        }
    }


    inputFile.close();
    outputFile.close();

    return 0;
}
