//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   COSEDA Technologies GmbH 2019
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//----------------------------------------------------------------------

#ifndef BUS_TRANS_H_
#define BUS_TRANS_H_

#include <uvm>

#include <iomanip>
#include <sstream>
#include <string>
#include <array>

//--------------------------------------------------------------------
// bus_trans
//--------------------------------------------------------------------
const unsigned int trans_size = 1000;

class bus_trans: public uvm::uvm_sequence_item {
public:

	bus_trans(const std::string& name = "bus_trans_seq_item") :
			uvm::uvm_sequence_item(name) {
	}

	~bus_trans() override {
	}

	UVM_OBJECT_UTILS(bus_trans);

	void do_copy( const uvm::uvm_object& rhs ) override
	{
		const bus_trans* rhs_ = dynamic_cast<const bus_trans*>(&rhs);
		if(rhs_ == nullptr)
		UVM_ERROR("do_copy", "cast failed, check type compatibility");

		uvm_sequence_item::do_copy(rhs);

		data = rhs_->data;
	}

	bool do_compare( const uvm::uvm_object& rhs, const uvm::uvm_comparer* comparer ) const override
	{
		const bus_trans* rhs_ = dynamic_cast<const bus_trans*>(&rhs);
		if(rhs_ == nullptr)
		UVM_FATAL("do_compare", "cast failed, check type compatibility");

		return ((data == rhs_->data));
	}

	void do_print( const uvm::uvm_printer& printer ) const override
	{
		unsigned int idx = 0;
		//for(const auto & i: data) {
			printer.print_field_int("data["+std::to_string(idx)+"]", data[0]); // ,i
		//	idx++;
		//}

	}

	std::string convert2string() const override
	{
		std::ostringstream str;
		for(const auto & i: data) {
			str << i << " ";
		}
		return str.str();
	}

	// data members
public:
	std::array<std::uint64_t, trans_size> data{};
};

#endif /* BUS_TRANS_H_ */
