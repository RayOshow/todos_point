
/**
 *  Tpoint main contract
 @Author : Ray OShow (raymond@todos.co.kr)
 @Company : Todos
 @brief: 
    It is a project for the Company reward point.
    Company reward point is connected to blockcahin tokens.
    It is pair and everyone can track all of trx.
    
 */
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/dispatcher.hpp>
#include <vector>
#include <eosiolib/crypto.hpp>
#include <eosiolib/transaction.hpp>

using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::print;
using eosio::name;
using std::string;
using std::vector;
using eosio::datastream;
using eosio::checksum256;
using eosio::transaction;

// main header
#include "todos_point.hpp"

// table
#include "table/account_table.hpp"
#include "table/qna_table.hpp"
#include "table/bucky_table.hpp"

// account
#include "account/account_control.hpp"

// qna
#include "qna/qna_define.hpp"
#include "qna/qna_control.hpp"

// bucky
#include "bucky/bucky_define.hpp"
#include "bucky/bucky_control.hpp"

class [[eosio::contract]] todos_point : public eosio::contract {

	public:

		using contract::contract;
		
		todos_point(name receiver, name code,  datastream<const char*> ds)
			: contract(receiver, code, ds)  
			, account_controller(_self)
			, qna_controller(_self) 
			, bucky_controller(_self){
		}

		/**
			Add user info. 
		**/
		[[eosio::action]]
		void signup(uint32_t user_seq)
		{	 			
			account_controller.signup(user_seq);
		}

		/**
			Put tokens into internal accounts. 
		**/
		[[eosio::action]]
		void transfer(name from, name to, asset quantity, string memo) {

			eosio_assert(quantity.is_valid(), ERROR_MSG_INVALID_ASSET);	

			// Recieve only
			if(from == _self) {
				return;
			}

			// [memo]
			// command-data
			string command;
			string data;

			/////////////////////////////////////////////////////////////////
			const size_t first_break = memo.find("-");

			// Just send
			if(first_break == string::npos){
				return;
			}

			command = memo.substr(0, first_break);
			data = memo.substr(first_break + 1);
			/////////////////////////////////////////////////////////////////

			if(command.empty()) {
				return;
			}

            // put-322
            // [command]-[user seq]
			if(command == COMMAND_NAME_PUT_TOKEN) {
				account_controller.charge_token(stoull(data, 0, 10), quantity);		
			}	
		}

		/**
			When user want to refund.
		**/
		[[eosio::action]]
		void refund(uint32_t user_seq, asset cost, string memo) {
			account_controller.charge_token(user_seq, cost);
		}

		/**
			When user use their tokens for purchaing something.
		**/
		[[eosio::action]]
		void consume(uint32_t user_seq, asset cost, string memo) {
			account_controller.uncharge_token(user_seq, cost);	
		}
		
		/**
			Set blockcahin id.
		**/
		[[eosio::action]]
		void setbcid(name blockchain_id, uint32_t user_seq)
		{
			account_controller.set_blockchain_id(blockchain_id, user_seq);		
		}

		/**
			User can claim and get his or her tokens.
		**/
		[[eosio::action]]
		void withdraw(name requestor, uint32_t user_seq, asset quantity) 
		{
			account_controller.withdraw_token(requestor,user_seq,quantity);
			send_token(_self, requestor, quantity, "withdraw" );						
		}

		/**
			Erase user data 
		**/
		[[eosio::action]]
		void eraseid(uint32_t user_seq) {
			account_controller.retire(user_seq);		
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// qna
		
		/**
			response to question.
		**/
		[[eosio::action]]
		void qnaresponse(uint32_t qna_seq, uint32_t user_seq, asset quantity, string answer_str) {
			qna_controller.response(qna_seq,user_seq,quantity,answer_str);
		}

		/**
			end question.
		**/
		[[eosio::action]]
		void qnaend(uint64_t qna_seq, string enc_pwd) {
			qna_controller.end(qna_seq, enc_pwd);					
		}	

		/**
			Erase qna 
		**/
		[[eosio::action]]
		void eraseqna(uint32_t qna_seq) {
			qna_controller.delete_info(qna_seq);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// bucky
		[[eosio::action]]
		void buckyreg(uint32_t bucky_seq, uint32_t max_participant, const std::vector<uint32_t>& picks) {
			bucky_controller.register_bucky(bucky_seq, max_participant, picks);
		}

		[[eosio::action]]
		void buckyjoin(uint32_t bucky_seq, uint32_t user_seq) {
			bucky_controller.join_bucky( bucky_seq,  user_seq);
		}		

		[[eosio::action]]
		void buckyend(uint32_t bucky_seq) {
			bucky_controller.finish_bucky( bucky_seq);
		}

		/**
			Erase bucky 
		**/
		[[eosio::action]]
		void erasebucky(uint32_t bucky_seq) {
			bucky_controller.delete_info(bucky_seq);
		}


	
	private:

		account_control account_controller;
		qna_control qna_controller;
		bucky_control bucky_controller;

		/**
			Send token by eosio.token contract outside.
			It is essential to have "eosio.code" permission.
		**/
		void send_token(name user1, name user2, asset quantity, string memo) {
			action(
				permission_level{_self, "active"_n},
				TOKEN_CONTRACT, 
				"transfer"_n,
				std::make_tuple(
					user1, 
					user2, 
					quantity, 
					memo)

			).send();
		}		
};

#define EOSIO_DISPATCH_CUSTOM( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
	if(code == "todoskrtoken"_n.value && action == "transfer"_n.value) {\
            eosio::execute_action(name(receiver), name(code), &todos_point::transfer);\
        }\
	else {\
		if( code == receiver ) { \
	     		switch (action) {\
               			EOSIO_DISPATCH_HELPER(TYPE, MEMBERS)\
        	    	}\
		}\
	}\
   } \
} \

EOSIO_DISPATCH_CUSTOM( todos_point, (signup)(transfer)(withdraw)(eraseid)(setbcid)(consume)(refund)(qnaresponse)(qnaend)(eraseqna)(buckyreg)(buckyend)(buckyjoin)(erasebucky))
