#pragma once


// 3% profit for qna account
#define QNA_WINNER_PROFIT_RATE 97 / 100

#define QNA_TEST_MODE_ON 1

// Token Contract name
#define TOKEN_CONTRACT "todoskrtoken"_n

// Winner symbol
#define QNA_POSITIVE_SYMBOL "Y"
#define QNA_NEGATIVE_SYMBOL "N"
#define QNA_NO_DECISION_SYMBOL "T"

// Comments for game result
#define QNA_WIN_SYMBOL "WIN"
#define QNA_LOSE_SYMBOL "LOSE"
#define QNA_TIE_SYMBOL "TIE"

#define QNA_WIN_COMMENT "[QnA] You're winner!"
#define QNA_TIE_COMMENT "[QnA] Tie, Return your bet"

// Database name
#define QNA_DB_NAME "qnainfos"_n

// Erorr code
#define QNA_ERROR_MSG_INVALID_ASSET "{\"CODE\":\"0001\":,\"MSG\":\"QNA_ERROR_MSG_INVALID_ASSET\"}"
#define QNA_ERROR_MSG_ALREADY_VOTE "{\"CODE\":\"0002\",\"MSG\":\"QNA_ERROR_MSG_ALREADY_VOTE\"}"
#define QNA_ERROR_MSG_SYMBOL_DIFF "{\"CODE\":\"0003\",\"MSG\":\"QNA_ERROR_MSG_SYMBOL_DIFF\"}"
#define QNA_ERROR_MSG_AMOUNT_IS_LOW "{\"CODE\":\"0004\",\"MSG\":\"QNA_ERROR_MSG_AMOUNT_IS_LOW\"}"
#define QNA_ERROR_MSG_NO_FIND_SEQ "{\"CODE\":\"0005\":\"MSG\":\"QNA_ERROR_MSG_NO_FIND_SEQ\"}"
#define QNA_ERROR_MSG_QNA_CLOSED "{\"CODE\":\"0006\",\"MSG\":\"QNA_ERROR_MSG_QNA_CLOSED\"}"
#define QNA_ERROR_MSG_NO_SEQ "{\"CODE\":\"0007\",\"MSG\":\"QNA_ERROR_MSG_NO_SEQ\"}"
#define QNA_ERROR_MSG_NO_ENC_PWD "{\"CODE\":\"0007\",\"MSG\":\"QNA_ERROR_MSG_NO_ENC_PWD\"}"
