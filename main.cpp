#include <iostream>
#include <vector>
#include <map>
#include <ctime>
using namespace std;


int ReadInt(int low, int high) {
    int value;
    cout << "Enter number (" << low << "-" << high << "): ";
    cin >> value;
    if (value < low || value > high) {
        cout << "Invalid input. Try again.\n";
        return ReadInt(low, high);
    }
    return value;
}

string CurrentDate() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[100];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}


class IShippable {
public:
    virtual string getName() const = 0;
    virtual double getWeight() const = 0;
    virtual ~IShippable() {}
};


class Product {
protected:
    string name;
    double price;
    int quantity;

public:
    Product(string name, double price, int qty) :
        name(name), price(price), quantity(qty) {}

    virtual ~Product() {}

    string getName() const { return name; }
    double getPrice() const { return price; }
    int getQuantity() const { return quantity; }

    void decreaseQuantity(int q) { quantity -= q; }

    virtual bool isExpired() const { return false; }
    virtual bool isShippable() const { return false; }
    virtual double getWeight() const { return 0.0; }

    virtual string info() const {
        return name + " | Price: $" + to_string(price) + " | Qty: " + to_string(quantity);
    }
};

class PerishableProduct : public Product {
    string expiryDate; // YYYY-MM-DD

public:
    PerishableProduct(string name, double price, int qty, string expiry)
        : Product(name, price, qty), expiryDate(expiry) {}

    bool isExpired() const override {
        return expiryDate < CurrentDate();
    }

    string info() const override {
        return Product::info() + " | Expiry: " + expiryDate;
    }
};

class ShippableProduct : public Product, public IShippable {
    double weight;

public:
    ShippableProduct(string name, double price, int qty, double weight)
        : Product(name, price, qty), weight(weight) {}

    bool isShippable() const override { return true; }
    double getWeight() const override { return weight; }

    string info() const override {
        return Product::info() + " | Weight: " + to_string(weight) + "kg";
    }

    string getName() const override { return name; }
};

class ShippablePerishableProduct : public PerishableProduct, public IShippable {
    double weight;

public:
    ShippablePerishableProduct(string name, double price, int qty, string expiry, double weight)
        : PerishableProduct(name, price, qty, expiry), weight(weight) {}

    bool isShippable() const override { return true; }
    double getWeight() const override { return weight; }

    string getName() const override { return name; }

    string info() const override {
        return PerishableProduct::info() + " | Weight: " + to_string(weight) + "kg";
    }
};


class CartItem {
public:
    Product* product;
    int quantity;

    CartItem(Product* p, int q) : product(p), quantity(q) {}
};

class Cart {
    vector<CartItem> items;

public:
    void addItem(Product* p, int qty) {
        if (qty <= 0 || qty > p->getQuantity()) {
            cout << "Invalid quantity.\n";
            return;
        }
        items.emplace_back(p, qty);
    }

    const vector<CartItem>& getItems() const {
        return items;
    }

    bool isEmpty() const {
        return items.empty();
    }

    void clear() {
        items.clear();
    }
};


class ShippingService {
public:
    static void ShipItems(const vector<IShippable*>& shippables) {
        cout << "Shipping " << shippables.size() << " items:\n";
        for (auto item : shippables) {
            cout << " - " << item->getName() << ", " << item->getWeight() << "kg\n";
        }
    }
};


class Customer {
    string name;
    double balance;
    Cart cart;

public:
    Customer(string name, double bal) : name(name), balance(bal) {}

    Cart& getCart() { return cart; }
    double getBalance() const { return balance; }
    string getName() const { return name; }

    void checkout() {
        if (cart.isEmpty()) {
            cout << "Error: Cart is empty.\n";
            return;
        }

        double subtotal = 0, shipping = 0;
        vector<IShippable*> toShip;

        for (const auto& item : cart.getItems()) {
            Product* p = item.product;

            if (p->isExpired()) {
                cout << "Error: Product '" << p->getName() << "' is expired.\n";
                return;
            }
            if (item.quantity > p->getQuantity()) {
                cout << "Error: Not enough quantity of '" << p->getName() << "'.\n";
                return;
            }

            subtotal += p->getPrice() * item.quantity;

            if (p->isShippable()) {
                for (int i = 0; i < item.quantity; ++i)
                    toShip.push_back(dynamic_cast<IShippable*>(p));
                shipping += 5.0 * item.quantity;
            }
        }

        double total = subtotal + shipping;

        if (total > balance) {
            cout << "Error: Insufficient balance.\n";
            return;
        }

        for (auto& item : cart.getItems())
            item.product->decreaseQuantity(item.quantity);

        balance -= total;

        cout << "=== Checkout Summary ===\n";
        cout << "Subtotal: $" << subtotal << "\n";
        cout << "Shipping: $" << shipping << "\n";
        cout << "Total Paid: $" << total << "\n";
        cout << "Remaining Balance: $" << balance << "\n";

        if (!toShip.empty())
            ShippingService::ShipItems(toShip);

        cart.clear();
    }
};


class Store {
    map<string, Product*> products;

public:
    ~Store() {
        for (auto& pair : products)
            delete pair.second;
    }

    void seedProducts() {
        products["cheese"] = new ShippablePerishableProduct("Cheese", 10.0, 10, "2025-12-01", 2.0);
        products["tv"] = new ShippableProduct("TV", 150.0, 5, 8.0);
        products["biscuit"] = new PerishableProduct("Biscuit", 2.0, 50, "2024-12-31");
        products["scratch"] = new Product("Mobile Scratch Card", 5.0, 100);
    }

    void listProducts() {
        cout << "\n--- Product List ---\n";
        int i = 1;
        for (auto& pair : products) {
            cout << i++ << ". " << pair.second->info() << "\n";
        }
    }

    vector<string> getProductNames() {
        vector<string> names;
        for (auto& pair : products)
            names.push_back(pair.first);
        return names;
    }

    Product* getProductByName(const string& name) {
        if (products.count(name))
            return products[name];
        return nullptr;
    }
};


int main() {
    Store store;
    store.seedProducts();

    Customer customer("Amir", 300.0);

    while (true) {
        cout << "\n==== Welcome " << customer.getName() << " ====\n";
        store.listProducts();

        auto names = store.getProductNames();
        cout << "Choose a product to add to cart (1-" << names.size() << "), or 0 to checkout: ";
        int ch = ReadInt(0, names.size());

        if (ch == 0) {
            customer.checkout();
            break;
        }

        string selected = names[ch - 1];
        Product* p = store.getProductByName(selected);

        cout << "Enter quantity to add: ";
        int qty;
        cin >> qty;

        customer.getCart().addItem(p, qty);
    }

    return 0;
}
