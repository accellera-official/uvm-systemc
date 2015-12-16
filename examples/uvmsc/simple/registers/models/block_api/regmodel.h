
#ifndef REGMODEL_H_
#define REGMODEL_H_

#include <systemc>
#include <uvm>


class reg_RO : public uvm::uvm_reg
{
    public:
    uvm::uvm_reg_field* F1;
    uvm::uvm_reg_field* F2;

    // Register reg_RO:
    // msb                                                                                           lsb
    // -------------------------------------------------------------------------------------------------
    // |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
    // |///////reserved////////|        F2 (RC)        |///////reserved////////|        F1 (RO)        |
    // -------------------------------------------------------------------------------------------------
    // Reset value:
    //   x  x  x  x  x  x  x  x  1  1  1  1  1  1  1  1  x  x  x  x  x  x  x  x  0  0  0  0  0  0  0  0
    //

    reg_RO( std::string name = "RO" ) : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
    {}

    virtual void build()
    {
        F1 = uvm::uvm_reg_field::type_id::create("F1");
        F1->configure(this, 8, 0, "RO", false, 0x00, true, false, true);
        F2 = uvm::uvm_reg_field::type_id::create("F2");
        F2->configure(this, 8, 16, "RC", false, 0xFF, true, false, true);
    }

    UVM_OBJECT_UTILS(reg_RO);

}; // class reg_RO


class reg_WO : public uvm::uvm_reg
{
    public:
    	 uvm::uvm_reg_field* F1; 
    	 uvm::uvm_reg_field* F2;

    // Register reg_WO:
    // msb                                                                                           lsb
    // -------------------------------------------------------------------------------------------------
    // |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
    // |///////reserved////////|             F2 (WO)               |        F1 (WO)        |/reserved//|
    // -------------------------------------------------------------------------------------------------
    // Reset value:
    //   x  x  x  x  x  x  x  x  1  1  0  0  1  1  0  0  1  1  0  0  1  0  1  0  1  0  1  0  x  x  x  x
    //

    reg_WO( std::string name = "WO") : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
    {}

    virtual void build()
    {
        F1 = uvm::uvm_reg_field::type_id::create("F1");
        F1->configure(this, 8, 4, "WO", true, 0xAA, true, true, true);
        F2 = uvm::uvm_reg_field::type_id::create("F2");
        F2->configure(this, 12, 12, "WO", true, 0xCCC, true, true, true);
    }

    UVM_OBJECT_UTILS(reg_WO);

}; // class reg_WO


class reg_RW : public uvm::uvm_reg
{
    public:
    /*rand*/ uvm::uvm_reg_field* F1; // TODO randomization
    /*rand*/ uvm::uvm_reg_field* F2;

    // Register reg_RW:
    // msb                                                                                           lsb
    // -------------------------------------------------------------------------------------------------
    // |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
    // |///////reserved////////|             F2 (RW)               |        F1 (RW)        |/reserved//|
    //  -------------------------------------------------------------------------------------------------
    // Reset value:
    //   x  x  x  x  x  x  x  x  1  0  1  0  1  0  1  0  1  0  1  0  1  0  1  1  1  1  0  1  x  x  x  x
    //  

    reg_RW( std::string name = "RW") : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
    {}




    virtual void build()
    {
        F1 = uvm::uvm_reg_field::type_id::create("F1");
        F1->configure(this, 8, 4, "RW", true, 0xAA, true, true, true);
        F2 = uvm::uvm_reg_field::type_id::create("F2");
        F2->configure(this, 12, 12, "WRC", true, 0xCCC, true, true, true);
    }

    UVM_OBJECT_UTILS(reg_RW);
}; // class reg_RW



class reg_WSRC : public uvm::uvm_reg
{
    public:
    /*rand*/ uvm::uvm_reg_field* F1; // TODO randomization
    /*rand*/ uvm::uvm_reg_field* F2;


    reg_WSRC( std::string name = "WSRC") : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
    {}

    virtual void build()
    {
        F1 = uvm::uvm_reg_field::type_id::create("F1");	
        F1 -> configure(this, 8, 4, "WSRC", false, 0xBB, true, true, true);
        F2 = uvm::uvm_reg_field::type_id::create("F2");
        F2 -> configure(this, 12, 12, "WSRC", true, 0xCCC, true, true, true);
    }

    UVM_OBJECT_UTILS(reg_WSRC);
}; // calss reg_WSRC



class block_B : public uvm::uvm_reg_block
{
    public:
    /*rand*/ reg_RO* R; // TODO randomization
    /*rand*/ reg_WO* W;
    /*rand*/ reg_RW* RW;
    /*rand*/ reg_WSRC* WSRC;




    // Register block block_B (part of map 'default_map').
    // Addr   msb                                                                                           lsb
    //        -------------------------------------------------------------------------------------------------
    //        |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
    //        -------------------------------------------------------------------------------------------------
    // 0x400  |///////reserved////////|        F2 (WSRC)      |///////reserved////////|        F1 (WSRC)      | reg_WSRC (WSRC)
    //        -------------------------------------------------------------------------------------------------
    // 0x300  |///////reserved////////|        F2 (RS)        |///////reserved////////|        F1 (WRS)        | reg_RS (RS)
    //        -------------------------------------------------------------------------------------------------
    // 0x0200 |///////reserved////////|        F2 (RW)        |///////reserved////////|        F1 (RW)        | reg_RW (RW)
    //        -------------------------------------------------------------------------------------------------
    // 0x0100 |///////reserved////////|        F2 (RC)        |///////reserved////////|        F1 (RO)        | reg_RO (RO)
    //        -------------------------------------------------------------------------------------------------
    // 0x0100 |///////reserved////////|             F2 (WO)               |        F1 (WO)        |/reserved//| reg_WO (WO)
    //        -------------------------------------------------------------------------------------------------
    // 0x0000 (base)

    block_B( std::string name = "B" ) : uvm::uvm_reg_block(name, uvm::UVM_NO_COVERAGE)
    {}

    virtual void build()
    {
        uvm::uvm_reg_addr_t base_addr = 0x0000;
        unsigned int n_bytes = 4;

        default_map = create_map("default_map", base_addr, n_bytes, uvm::UVM_BIG_ENDIAN);

        R = reg_RO::type_id::create("RO");
        R->configure(this, NULL, "RO_reg");
        R->build();

        W = reg_WO::type_id::create("WO");
        W->configure(this, NULL, "WO_reg");
        W->build();

        RW = reg_RW::type_id::create("RW");
        RW->configure(this, NULL, "");
        RW->build();

        WSRC = reg_WSRC::type_id::create("WSRC");
        WSRC->configure(this, NULL, "WSRC_reg");
        WSRC->build();



        default_map->add_reg(R,    0x100, "RO");
        default_map->add_reg(W,    0x100, "WO");
        default_map->add_reg(RW,   0x200, "RW");
        default_map->add_reg(WSRC, 0x400, "RW");

	RW->clear_hdl_path();
	RW->add_hdl_path_slice("W_reg1", 0, 8,0,"RTL");

	lock_model();
    }

    UVM_OBJECT_UTILS(block_B);

}; // class block_B


#endif // REGMODEL_H_
