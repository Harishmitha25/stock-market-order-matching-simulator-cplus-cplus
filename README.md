# Stock Market Order Matching Simulator - README

## Overview
The `Stock Market Order Matching Simulator` is a C++ program that simulates the matching and execution of buy and sell trade orders. Each order can be a **market** or **limit** order, and orders are matched in real-time as they are read from a file. Matching rules prioritise market orders, price limits, and arrival times to simulate a simplified but functional trading system.

### Key Features
1. **Market & Limit Orders**: Supports both types of trading instructions.
2. **Priority-Based Matching**: Matches orders based on price, type, and arrival order.
3. **Partial Executions**: Allows divisible matching and handles residual orders.
4. **Real-Time Execution**: Processes orders as they are read, with immediate matching.
5. **Price Determination Rules**: Calculates execution price based on order types.
6. **Order Book Maintenance**: Maintains and displays current pending buy and sell orders.
7. **Formatted Output**: Writes execution results and unfulfilled orders to file with strict formatting.

### Order Format
Each order contains the following fields:
- `order_id`: Unique alphanumeric string identifying the order.
- `type`: `'B'` for buy, `'S'` for sell.
- `quantity`: Number of shares to buy or sell.
- `price`: *(optional)* Limit price; if omitted, the order is a market order.

Example input:
```
ord001 B 100 9.75
ord002 S 50
ord003 S 100 10.00
```

---

## Simulator Structure

The program maintains two order books:
- **Buy Order Book**: Prioritises higher price, then earlier arrival.
- **Sell Order Book**: Prioritises lower price, then earlier arrival.

Each new order is matched immediately with the highest priority order from the opposite side if matching conditions are met.

### Matching Conditions
A buy order `B` matches a sell order `S` if:
- `B.price ≥ S.price`, or
- either one is a market order

### Priority Rules
- **Market orders** have the highest priority.
- **Limit orders** are prioritised by:
  - Higher limit price (for buy), lower limit price (for sell)
  - Earlier arrival in case of tie

### Execution Price Rules
1. Both orders are limit → price of earlier order  
2. One market, one limit → price of the limit order  
3. Both market → price of previous transaction (initial price is read from file)

### Residual Orders
If matched quantities are unequal:
- The smaller quantity is executed
- The remaining quantity becomes a new order with the same arrival time
- Residual orders are matched immediately, recursively if needed

---

## Class Functions

### Order Class
#### Constructor
```cpp
Order(string id, char type, int quantity, optional<float> price)
```
Initializes an order with its basic fields.

### OrderBook Class
Maintains either the buy or sell book.

#### Insert
```cpp
void insert(const Order& order)
```
Adds a new order, maintaining priority.

#### Match
```cpp
vector<Execution> match(Order& incoming, float& lastPrice)
```
Matches an order against the book, returning executed trades and updating the last traded price.

### Simulator Class
#### Load and Run
```cpp
void processFile(const string& inputFile)
```
Reads and processes all orders from file and outputs the result.

---

## Examples

### Input File
```
10.00
ord001 B 50 9.75
ord002 S 30
ord003 S 40 9.50
```

### Console Output
```
Last trading price: 10.00

Buy Orders           | Sell Orders
---------------------|---------------------
ord001 9.75 50       | ord002 M    30
                     | ord003 9.50 40
```

### Output File (output1.txt)
```
order ord001 30 shares purchased at price 9.50
order ord003 30 shares sold at price 9.50
order ord001 20 shares purchased at price 10.00
order ord002 20 shares sold at price 10.00
order ord002 10 shares unexecuted
```

---

## Implementation Details

### Key Components to Implement
- **Order Class**: Stores ID, type, quantity, price, and timestamp.
- **OrderBook Class**: Handles insertion and priority ordering of pending orders.
- **Matching Logic**: Matches incoming orders with best-fit existing orders.
- **Execution Record**: Stores matched trades with ID, quantity, and price.
- **File Handling**:
  - Reads from `input#.txt`
  - Writes to `output#.txt`

### Precision and Formatting
- Use `std::fixed` and `std::setprecision(2)` for all price outputs.
- Input/output file names must correspond (`input1.txt → output1.txt`).

---

## Notes
- Matching must be done **immediately** upon reading each order.
- Orders are assumed **divisible** unless otherwise specified.
- Final unexecuted orders must be recorded exactly as per format.
- Residual orders maintain original priority and are not pushed to the end.
- The screen output format is flexible, but the file output must match exactly.

---
