#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>
#include <limits>
#include <sstream>
#include <cmath>
using namespace std;
class BankingException : public runtime_error {
public:
    BankingException(const string &message) : runtime_error(message) {}
};
string getCurrentDateTime() {
    time_t now = time(0);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buffer);
}
class Transaction {
private:
    static int nextTransactionId;
    int transactionId;
    string type;
    double amount;
    double balanceAfter;
    string description;
    string dateTime;
public:
    Transaction(string type, double amount, double balanceAfter, string description)
        : type(type), amount(amount), balanceAfter(balanceAfter),
          description(description) {
        transactionId = ++nextTransactionId;
        dateTime = getCurrentDateTime();
    }
    int getTransactionId() const {
        return transactionId;
    }
    string getType() const {
        return type;
    }
    double getAmount() const {
        return amount;
    }
    double getBalanceAfter() const {
        return balanceAfter;
    }
    string getDescription() const {
        return description;
    }
    string getDateTime() const {
        return dateTime;
    }
};
int Transaction::nextTransactionId = 1000;
class Customer {
private:
    static int nextCustomerId;
    int customerId;
    string name;
    string email;
    string phone;
    string address;
    vector<int> accountIds;
public:
    Customer(const string &name, const string &email,
             const string &phone, const string &address)
        : name(name), email(email), phone(phone), address(address) {
        if (name.empty() || email.empty() || phone.empty() || address.empty()) {
            throw BankingException("Customer details cannot be empty.");
        }
        if (email.find('@') == string::npos || email.find('.') == string::npos) {
            throw BankingException("Invalid email address.");
        }
        if (phone.size() != 10) {
            throw BankingException("Phone number must contain exactly 10 digits.");
        }
        for (char digit : phone) {
            if (!isdigit(digit)) {
                throw BankingException("Phone number must contain digits only.");
            }
        }
        customerId = ++nextCustomerId;
    }
    int getCustomerId() const {
        return customerId;
    }
    string getName() const {
        return name;
    }
    string getEmail() const {
        return email;
    }
    void addAccount(int accountId) {
        accountIds.push_back(accountId);
    }
    const vector<int>& getAccountIds() const {
        return accountIds;
    }
    void display() const {
        cout << "\nCustomer ID   : " << customerId << endl;
        cout << "Name          : " << name << endl;
        cout << "Email         : " << email << endl;
        cout << "Phone         : " << phone << endl;
        cout << "Address       : " << address << endl;
        cout << "Total Accounts: " << accountIds.size() << endl;
    }
};
int Customer::nextCustomerId = 100;
class Account {
protected:
    static int nextAccountId;
    int accountId;
    string accountNumber;
    double balance;
    vector<Transaction> transactions;
    void addTransaction(const string &type, double amount,
                        double balanceAfter, const string &description) {
        transactions.push_back(
            Transaction(type, amount, balanceAfter, description)
        );
    }
public:
    Account(double openingBalance)
        : balance(openingBalance) {
        if (openingBalance < 0) {
            throw BankingException("Opening balance cannot be negative.");
        }
        accountId = ++nextAccountId;
        stringstream accountNumberStream;
        accountNumberStream << "SB" << accountId;
        accountNumber = accountNumberStream.str();
        addTransaction("ACCOUNT_OPENING", openingBalance, balance,
                       "Account opened");
    }
    virtual ~Account() {}
    int getAccountId() const {
        return accountId;
    }
    string getAccountNumber() const {
        return accountNumber;
    }
    double getBalance() const {
        return balance;
    }
    void deposit(double amount) {
        if (amount <= 0) {
            throw BankingException("Deposit amount must be greater than zero.");
        }
        balance += amount;
        addTransaction("DEPOSIT", amount, balance, "Cash deposit");
        cout << fixed << setprecision(2);
        cout << "\nRs. " << amount << " deposited." << endl;
        cout << "Balance: Rs. " << balance << endl;
    }
    virtual void withdraw(double amount) {
        if (amount <= 0) {
            throw BankingException("Withdrawal amount must be greater than zero.");
        }
        if (amount > getAvailableBalance()) {
            throw BankingException("Insufficient balance for withdrawal.");
        }
        balance -= amount;
        addTransaction("WITHDRAWAL", amount, balance, "Cash withdrawal");
        cout << fixed << setprecision(2);
        cout << "\nRs. " << amount << " withdrawn." << endl;
        cout << "Balance: Rs. " << balance << endl;
    }
    virtual double getAvailableBalance() const {
        return balance;
    }
    virtual string accountType() const = 0;
    virtual void displayDetails() const {
        cout << fixed << setprecision(2);
        cout << "\nAccount ID     : " << accountId << endl;
        cout << "Account Number : " << accountNumber << endl;
        cout << "Account Type   : " << accountType() << endl;
        cout << "Balance        : Rs. " << balance << endl;
        cout << "Available Funds: Rs. " << getAvailableBalance() << endl;
    }
    void displayStatement() const {
        displayDetails();
        cout << "\n---------------- TRANSACTION HISTORY ----------------" << endl;
        if (transactions.empty()) {
            cout << "No transactions found." << endl;
            return;
        }
        cout << left
             << setw(8) << "ID"
             << setw(22) << "Date/Time"
             << setw(18) << "Type"
             << setw(14) << "Amount"
             << setw(14) << "Balance"
             << "Description" << endl;
        cout << string(105, '-') << endl;
        for (const Transaction &transaction : transactions) {
            cout << left
                 << setw(8) << transaction.getTransactionId()
                 << setw(22) << transaction.getDateTime()
                 << setw(18) << transaction.getType()
                 << ("Rs. " + to_string(transaction.getAmount())).substr(0, 13)
                 << setw(14) << ""
                 << setw(14) << transaction.getBalanceAfter()
                 << transaction.getDescription() << endl;
        }
    }
    friend ostream& operator<<(ostream &output, const Account &account);
};
int Account::nextAccountId = 1000;
ostream& operator<<(ostream &output, const Account &account) {
    output << fixed << setprecision(2);
    output << "[" << account.accountType() << "] "
           << account.accountNumber
           << " | Balance: Rs. " << account.getBalance();
    return output;
}
class SavingsAccount : public Account {
private:
    double minimumBalance;
    double annualInterestRate;
    static const double PENALTY_AMOUNT;
public:
    SavingsAccount(double openingBalance, double minimumBalance = 1000.0,
                   double annualInterestRate = 4.0)
        : Account(openingBalance),
          minimumBalance(minimumBalance),
          annualInterestRate(annualInterestRate) {
        if (openingBalance < minimumBalance) {
            throw BankingException(
                "Opening balance is below the savings minimum balance."
            );
        }
    }
    string accountType() const override {
        return "SAVINGS";
    }
    void withdraw(double amount) override {
        if (amount <= 0) {
            throw BankingException("Withdrawal amount must be greater than zero.");
        }
        if (amount > getAvailableBalance()) {
            throw BankingException("Insufficient balance for withdrawal.");
        }
        balance -= amount;
        if (balance < minimumBalance) {
            balance -= PENALTY_AMOUNT;
            addTransaction("PENALTY", PENALTY_AMOUNT, balance,
                           "Minimum balance penalty applied");
            cout << "\nMinimum balance went below the limit." << endl;
            cout << "Penalty: Rs. " << PENALTY_AMOUNT << endl;
        }
        addTransaction("WITHDRAWAL", amount, balance, "Cash withdrawal");
        cout << fixed << setprecision(2);
        cout << "Rs. " << amount << " withdrawn." << endl;
        cout << "Balance: Rs. " << balance << endl;
    }
    void creditInterest() {
        double interest = (balance * annualInterestRate) / 100.0;
        balance += interest;
        addTransaction("INTEREST", interest, balance,
                       "Savings interest added");
        cout << fixed << setprecision(2);
        cout << "\nInterest added: Rs. " << interest << endl;
        cout << "Balance: Rs. " << balance << endl;
    }
    void displayDetails() const override {
        Account::displayDetails();
        cout << "Minimum Balance : Rs. " << minimumBalance << endl;
        cout << "Interest Rate   : " << annualInterestRate << "% per annum" << endl;
    }
};
const double SavingsAccount::PENALTY_AMOUNT = 100.0;
class CurrentAccount : public Account {
private:
    double overdraftLimit;
public:
    CurrentAccount(double openingBalance, double overdraftLimit = 0.0)
        : Account(openingBalance), overdraftLimit(overdraftLimit) {

        if (overdraftLimit < 0) {
            throw BankingException("Overdraft limit cannot be negative.");
        }
    }
    string accountType() const override {
        return "CURRENT";
    }
    double getAvailableBalance() const override {
        return balance + overdraftLimit;
    }
    void displayDetails() const override {
        Account::displayDetails();
        cout << "Overdraft Limit : Rs. " << overdraftLimit << endl;
    }
};
class FixedDepositAccount : public Account {
private:
    double annualInterestRate;
    double tenureYears;
    bool matured;
public:
    FixedDepositAccount(double principal, double annualInterestRate,
                        double tenureYears)
        : Account(principal),
          annualInterestRate(annualInterestRate),
          tenureYears(tenureYears),
          matured(false) {
        if (principal <= 0) {
            throw BankingException("Fixed deposit principal must be greater than zero.");
        }
        if (annualInterestRate <= 0) {
            throw BankingException("Interest rate must be greater than zero.");
        }
        if (tenureYears <= 0) {
            throw BankingException("Tenure must be greater than zero.");
        }
    }
    string accountType() const override {
        return "FIXED_DEPOSIT";
    }
    double calculateMaturityAmount() const {
        return balance * pow(1 + annualInterestRate / 100.0, tenureYears);
    }
    void withdraw(double amount) override {
        throw BankingException(
            "Direct withdrawal is not allowed from a fixed deposit account."
        );
    }
    void closeDeposit() {
        if (matured) {
            throw BankingException("Fixed deposit has already been closed.");
        }
        double maturityAmount = calculateMaturityAmount();
        double penalty = maturityAmount * 0.01;
        double finalAmount = maturityAmount - penalty;
        matured = true;
        addTransaction("FD_CLOSURE", finalAmount, finalAmount,
                       "Fixed deposit closed before maturity with penalty");
        cout << fixed << setprecision(2);
        cout << "\nFixed deposit closed before maturity." << endl;
        cout << "Maturity amount     : Rs. " << maturityAmount << endl;
        cout << "Premature penalty   : Rs. " << penalty << endl;
        cout << "Final payout amount : Rs. " << finalAmount << endl;
    }
    void displayDetails() const override {
        Account::displayDetails();
        cout << "Interest Rate   : " << annualInterestRate << "% per annum" << endl;
        cout << "Tenure          : " << tenureYears << " year(s)" << endl;
        cout << "Maturity Amount : Rs. " << calculateMaturityAmount() << endl;
    }
};
class Bank {
private:
    vector<Customer> customers;
    vector<Account*> accounts;
    int findCustomerIndex(int customerId) const {
        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i].getCustomerId() == customerId) {
                return static_cast<int>(i);
            }
        }
        throw BankingException("Customer not found.");
    }
    Account* findAccount(int accountId) const {
        for (Account *account : accounts) {
            if (account->getAccountId() == accountId) {
                return account;
            }
        }
        throw BankingException("Account not found.");
    }
public:
    Bank() {}
    ~Bank() {
        for (Account *account : accounts) {
            delete account;
        }
        accounts.clear();
    }
    int registerCustomer(const string &name, const string &email,
                         const string &phone, const string &address) {
        for (const Customer &customer : customers) {
            if (customer.getEmail() == email) {
                throw BankingException("Customer with this email already exists.");
            }
        }
        Customer newCustomer(name, email, phone, address);
        customers.push_back(newCustomer);
        cout << "\nCustomer added." << endl;
        cout << "Customer ID: " << newCustomer.getCustomerId() << endl;
        return newCustomer.getCustomerId();
    }
    void displayCustomers() const {
        if (customers.empty()) {
            cout << "\nNo customers found." << endl;
            return;
        }
        for (const Customer &customer : customers) {
            customer.display();
        }
    }
    int createSavingsAccount(int customerId, double openingBalance,
                             double minimumBalance, double interestRate) {
        int index = findCustomerIndex(customerId);
        SavingsAccount *account =
            new SavingsAccount(openingBalance, minimumBalance, interestRate);
        accounts.push_back(account);
        customers[index].addAccount(account->getAccountId());
        account->displayDetails();
        return account->getAccountId();
    }
    int createCurrentAccount(int customerId, double openingBalance,
                             double overdraftLimit) {
        int index = findCustomerIndex(customerId);
        CurrentAccount *account =
            new CurrentAccount(openingBalance, overdraftLimit);
        accounts.push_back(account);
        customers[index].addAccount(account->getAccountId());
        account->displayDetails();
        return account->getAccountId();
    }
    int createFixedDepositAccount(int customerId, double principal,
                                  double interestRate, double tenureYears) {
        int index = findCustomerIndex(customerId);
        FixedDepositAccount *account =
            new FixedDepositAccount(principal, interestRate, tenureYears);
        accounts.push_back(account);
        customers[index].addAccount(account->getAccountId());
        account->displayDetails();
        return account->getAccountId();
    }
    void depositMoney(int accountId, double amount) {
        Account *account = findAccount(accountId);
        account->deposit(amount);
    }
    void withdrawMoney(int accountId, double amount) {
        Account *account = findAccount(accountId);
        account->withdraw(amount);
    }
    void transferMoney(int senderAccountId, int receiverAccountId, double amount) {
        if (amount <= 0) {
            throw BankingException("Transfer amount must be greater than zero.");
        }
        if (senderAccountId == receiverAccountId) {
            throw BankingException("Sender and receiver accounts must be different.");
        }
        Account *sender = findAccount(senderAccountId);
        Account *receiver = findAccount(receiverAccountId);
        if (amount > sender->getAvailableBalance()) {
            throw BankingException("Insufficient balance for transfer.");
        }
        sender->withdraw(amount);
        receiver->deposit(amount);
        cout << "\nTransfer complete." << endl;
    }
    void creditSavingsInterest(int accountId) {
        Account *account = findAccount(accountId);
        SavingsAccount *savingsAccount =
            dynamic_cast<SavingsAccount*>(account);
        if (!savingsAccount) {
            throw BankingException("Interest can be credited only to a savings account.");
        }
        savingsAccount->creditInterest();
    }
    void closeFixedDeposit(int accountId) {
        Account *account = findAccount(accountId);
        FixedDepositAccount *fixedDeposit =
            dynamic_cast<FixedDepositAccount*>(account);
        if (!fixedDeposit) {
            throw BankingException("This account is not a fixed deposit.");
        }
        fixedDeposit->closeDeposit();
    }
    void displayAllAccounts() const {
        if (accounts.empty()) {
            cout << "\nNo accounts found." << endl;
            return;
        }
        for (const Account *account : accounts) {
            cout << *account << endl;
        }
    }
    void displayStatement(int accountId) const {
        Account *account = findAccount(accountId);
        account->displayStatement();
    }
};
class BankingApplication {
private:
    Bank bank;
    int readInteger(const string &message) {
        int value;
        cout << message;
        cin >> value;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw BankingException("Invalid input. Please enter an integer.");
        }
        return value;
    }
    double readDouble(const string &message) {
        double value;
        cout << message;
        cin >> value;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw BankingException("Invalid input. Please enter a number.");
        }
        return value;
    }
    string readString(const string &message) {
        string value;
        cout << message;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, value);
        return value;
    }
    void registerCustomer() {
        cout << "\n----- CUSTOMER REGISTRATION -----" << endl;
        string name = readString("Enter customer name: ");
        string email = readString("Enter email: ");
        string phone = readString("Enter 10-digit phone number: ");
        string address = readString("Enter address: ");
        bank.registerCustomer(name, email, phone, address);
    }
    void createAccount() {
        cout << "\n----- CREATE BANK ACCOUNT -----" << endl;
        int customerId = readInteger("Enter customer ID: ");
        cout << "1. Savings Account" << endl;
        cout << "2. Current Account" << endl;
        cout << "3. Fixed Deposit Account" << endl;
        int choice = readInteger("Enter account type: ");
        double openingBalance = readDouble("Enter opening balance: ");
        int accountId;
        if (choice == 1) {
            double minimumBalance = readDouble("Enter minimum balance: ");
            double interestRate = readDouble("Enter annual interest rate (%): ");
            accountId = bank.createSavingsAccount(
                customerId, openingBalance, minimumBalance, interestRate
            );
        } else if (choice == 2) {
            double overdraftLimit = readDouble("Enter overdraft limit: ");
            accountId = bank.createCurrentAccount(
                customerId, openingBalance, overdraftLimit
            );
        } else if (choice == 3) {
            double interestRate = readDouble("Enter annual interest rate (%): ");
            double tenureYears = readDouble("Enter tenure in years: ");
            accountId = bank.createFixedDepositAccount(
                customerId, openingBalance, interestRate, tenureYears
            );
        } else {
            throw BankingException("Invalid account type choice.");
        }
        cout << "Account created. ID: "
             << accountId << endl;
    }
    void depositMoney() {
        int accountId = readInteger("\nEnter account ID: ");
        double amount = readDouble("Enter deposit amount: ");
        bank.depositMoney(accountId, amount);
    }
    void withdrawMoney() {
        int accountId = readInteger("\nEnter account ID: ");
        double amount = readDouble("Enter withdrawal amount: ");
        bank.withdrawMoney(accountId, amount);
    }
    void transferMoney() {
        int senderId = readInteger("\nEnter sender account ID: ");
        int receiverId = readInteger("Enter receiver account ID: ");
        double amount = readDouble("Enter transfer amount: ");
        bank.transferMoney(senderId, receiverId, amount);
    }
    void creditInterest() {
        int accountId = readInteger("\nEnter savings account ID: ");
        bank.creditSavingsInterest(accountId);
    }
    void closeFixedDeposit() {
        int accountId = readInteger("\nEnter fixed deposit account ID: ");
        bank.closeFixedDeposit(accountId);
    }
    void showStatement() {
        int accountId = readInteger("\nEnter account ID: ");
        bank.displayStatement(accountId);
    }
public:
    void run() {
        while (true) {
            cout << "\n========== BANKING SYSTEM ==========" << endl;
            cout << "1. Register Customer" << endl;
            cout << "2. View Customers" << endl;
            cout << "3. Create Account" << endl;
            cout << "4. View All Accounts" << endl;
            cout << "5. Deposit Money" << endl;
            cout << "6. Withdraw Money" << endl;
            cout << "7. Transfer Money" << endl;
            cout << "8. Credit Savings Interest" << endl;
            cout << "9. Close Fixed Deposit" << endl;
            cout << "10. View Account Statement" << endl;
            cout << "11. Exit" << endl;
            int choice;
            cout << "Enter your choice: ";
            cin >> choice;
            try {
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    throw BankingException("Invalid menu choice.");
                }
                switch (choice) {
                    case 1:
                        registerCustomer();
                        break;
                    case 2:
                        bank.displayCustomers();
                        break;
                    case 3:
                        createAccount();
                        break;
                    case 4:
                        bank.displayAllAccounts();
                        break;
                    case 5:
                        depositMoney();
                        break;
                    case 6:
                        withdrawMoney();
                        break;
                    case 7:
                        transferMoney();
                        break;
                    case 8:
                        creditInterest();
                        break;
                    case 9:
                        closeFixedDeposit();
                        break;
                    case 10:
                        showStatement();
                        break;
                    case 11:
                        cout << "\nExiting banking system." << endl;
                        return;
                    default:
                        cout << "\nInvalid choice. Please try again." << endl;
                }
            } catch (const BankingException &error) {
                cout << "\nError: " << error.what() << endl;
            } catch (const exception &error) {
                cout << "\nUnexpected error: " << error.what() << endl;
            }
        }
    }
};
int main() {
    BankingApplication application;
    application.run();
    return 0;
}