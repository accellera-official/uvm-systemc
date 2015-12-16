

#ifndef TB_TEST_H_
#define TB_TEST_H_

#include <systemc>
#include <uvm>

#include "regmodel.h"
#include "tb_env.h"

class tb_test : public uvm::uvm_test
{
    public:
    tb_test( uvm::uvm_component_name name ) : uvm::uvm_test(name)
    {
        set_report_verbosity_level(uvm::UVM_FULL);
    }

    UVM_COMPONENT_UTILS(tb_test);

    virtual void run_phase( uvm::uvm_phase& phase )
    {
        tb_env* env;
        uvm::uvm_status_e   status;
        uvm::uvm_reg_data_t data;

        phase.raise_objection(this);

        env = dynamic_cast<tb_env*>(uvm::uvm_root::get()->find("env"));

        if ( env == NULL )
            UVM_FATAL("test", "Cannot find tb_env");

        UVM_INFO("Test", "Reset RegModel...", uvm::UVM_NONE);
        env->regmodel->reset();

        uvm::uvm_sequence_base* seq = new uvm::uvm_sequence_base("seq");

        UVM_INFO("Test", "Demonstrating register prototype with differetnt field accessess...", uvm::UVM_NONE);

        env->regmodel->R->read(status, data, uvm::UVM_DEFAULT_PATH, NULL, seq);
        env->regmodel->W->write(status, 0xDEADBEEF, uvm::UVM_DEFAULT_PATH, NULL, seq);
        env->regmodel->RW->read(status, data, uvm::UVM_DEFAULT_PATH, NULL, seq);

	//writing directed values to
	UVM_INFO("Test", "Writing directed value to register by using Backdoor Access...", uvm::UVM_NONE); 
        env->regmodel->RW->write(status, 0xBAAAAAAB, uvm::UVM_BACKDOOR, NULL, seq);

	env->regmodel->RW->read(status, data, uvm::UVM_BACKDOOR, NULL, seq);


	env->regmodel->WSRC->read(status, data, uvm::UVM_DEFAULT_PATH, NULL, seq);
	env->regmodel->WSRC->write(status, data, uvm::UVM_DEFAULT_PATH, NULL, seq);
	
        UVM_INFO("Test", "Done.", uvm::UVM_NONE);

        phase.drop_objection(this);
    }

}; // class tb_test

#endif // TB_TEST_H_
