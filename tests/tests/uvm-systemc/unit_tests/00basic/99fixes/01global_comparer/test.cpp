#include <iostream>
#include <string>
#include <uvm>

using namespace uvm;

class seq_item : public uvm::uvm_sequence_item {

public:

	seq_item(const std::string& name = "test_item"):
		uvm::uvm_sequence_item(name), value(0) {}

	UVM_OBJECT_UTILS(seq_item);

	void do_print(const uvm::uvm_printer& printer) const override {
		printer.print_field_int("value", value);
	}

	bool do_compare( const uvm_object& rhs, const uvm_comparer* comparer) const override {
		bool out = true;

		const seq_item* rhs_ = dynamic_cast<const seq_item*>(&rhs);
		if (rhs_ == nullptr) {
			UVM_FATAL("do_compare", "cast failed, check type compatibility");
		}

		out = comparer->compare_field_int("", value, rhs_->value, 32, UVM_DEC);

		return (out);
	}

	int32_t value;

};

int sc_main (int argc, char** argv) {
	seq_item a, b, c;

	a.value = 0x1;
	b.value = 0x2;
	c.value = 0x1;

	a.print();
	b.print();
	c.print();
	
	bool a_compared_b = a.compare(b);
	bool a_compared_c = a.compare(c);

	std::cout << "comparison result : " << std::boolalpha << a_compared_b << std::endl;
	std::cout << "comparison result : " << std::boolalpha << a_compared_c << std::endl;

	return 0;
}
