/*
 *	BANK.C
 *
 *	A generic banking system written for Mordor.
 *
 *	Written by Christopher Beard, December, 1996
 *  		nexus.mudservices.com port 2000 
 *  Modified, expanded and debugged by Roy Wilson, June, 1998
 *  		styx@styx.darbonne.com
 *	
 */

#include "mstruct.h"
#include "mextern.h"

/*************************************************************************/
/*                            bank_balance                               */
/*************************************************************************/
/*	This will display a player's account balance if he is in a bank. */

int bank_balance(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	int		fee;

	int fd = ply_ptr->fd;

	if(!F_ISSET(ply_ptr->parent_rom, RPBANK)) {
		print(fd, "This is not a bank window.\n");
		if(ply_ptr->class < DM)
			return(PROMPT);
	}

	if(F_ISSET(ply_ptr, PLOANB)) {
		print(fd, "Your loan amount is $%ld\n", ply_ptr->bank_bal);
		return(PROMPT);
	}

	if(ply_ptr->bank_bal < 1) {
		ply_ptr->bank_bal = 0;	
		print(fd, "You do not have an account here.\n");
	}
	else {
		if(ply_ptr->bank_bal <= 100) {
			print(fd, "Your bank balance was $%ld.\n", ply_ptr->bank_bal); 
			print(fd, "Bank fees have reduced it to $0.0\n");
			ply_ptr->bank_bal = 0;
		}
		else {	
			print(fd, 
			"There is a $10 bank fee for checking your balance.\n");
			ply_ptr->bank_bal -= 10;
			print(fd, "Your bank balance is $%ld in gold.\n", 
			ply_ptr->bank_bal);
		}
	}
	return(PROMPT);	
}

/*************************************************************************/
/*                            bank_withdraw                              */
/*************************************************************************/
/*	This command allows players to withdraw from their bank accounts.*/
/*  It checks to ensure they are standing within the bank, that they 	 */
/*  are not trying to hack the account, and that they have enough money. */

int bank_withdraw(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	int fd = ply_ptr->fd;
	long amt;

	if(!F_ISSET(ply_ptr->parent_rom, RPBANK)) {
		print(fd, "This is not a bank window.\n");
		if(ply_ptr->class < CARETAKER)
			return(PROMPT);
	}

	if(ply_ptr->bank_bal < 0) {
		print(fd, 
			"You have an outstanding loan and cannot withdraw funds.\n");
		print(fd, "You must repay $%ld in gold.\n", -ply_ptr->bank_bal);
		return(PROMPT);
	}

	if(cmnd->num < 2) {
		print(fd, "You must specify how much you wish to withdraw.\n");
		print(fd, "Don't forget to use a $ sign with the amount.\n");
		return(PROMPT);
	} 

	if(!strcmp(cmnd->str[1], "all")) {
		amt = ply_ptr->bank_bal;
		print(fd, "This will close your account.  Thank you for your patronage.\n");
	}

	else if(cmnd->str[1][0] != '$') {
		print(fd, "The amount must be preceded by a $.\n");
		return(PROMPT);
	}

	else if(cmnd->str[1][0] == '$') {
			amt = atol(&cmnd->str[1][1]);
			if(amt < 1) {
			print(fd, 
				"What are you trying to pull?  I should tell %s.\n", DMNAME);
			return(PROMPT);
		}
	}

	if(amt > 10000000) {
		print(fd, "Sorry, that amount is to large for the bank to handle.\n");
		return(PROMPT);
	}

	if(ply_ptr->bank_bal < amt) {
		print(fd, "Sorry, you don't have that much in your account.\n");
		print(fd, "Your balance is only $%ld\n", ply_ptr->bank_bal);
		return(PROMPT);
	}

	print(fd, "You withdraw $%ld in gold.\n", amt);
	ply_ptr->gold += amt;
	ply_ptr->bank_bal -= amt;
	print(fd, "Your bank balance is $%ld in gold.\n", ply_ptr->bank_bal);
	savegame(ply_ptr, 0);
	return(PROMPT);
}

/****************************************************************************/
/*                            bank_deposit                                  */
/****************************************************************************/
/*	This command allows a player to deposit into a bank account.	    */
/*	It does checking for hacking, and imposes a maximum on the savings. */

int bank_deposit(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	long fee;
	long amt;
	int fd = ply_ptr->fd;

	if(!F_ISSET(ply_ptr->parent_rom, RPBANK)) {
		print(fd, "This is not a bank window.\n");
		if(ply_ptr->class < CARETAKER)
		return(PROMPT);
	}

	if(cmnd->num < 2) {
		print(fd, "You must specify how much you wish to deposit.\n");
		print(fd, "Don't forget to use a $ sign with the amount.\n");
		return(PROMPT);
	}

	if(cmnd->str[1][0] != '$') {
		print(fd, "The amount must be preceded by a $.\n");
		return(PROMPT);
	}

	amt = atol(&cmnd->str[1][1]);
	if(amt < 1) {
		print(fd, "Get real.\n");
		print(fd, "What are you trying to pull?  I should tell %s.\n",
			DMNAME);
		return(PROMPT);
	}


	if(amt < 100) {
		print(fd, "Deposit must be at least 100 gold pieces.\n");
		return(PROMPT);
	}

	if(amt > 10000000) {
		print(fd, "We do not accept deposits that large.\n");
		return(PROMPT);
	}

	if(ply_ptr->gold < amt) {
		print(fd, "You don't have that much gold.\n");
		return(PROMPT);
	}

	if((amt + ply_ptr->bank_bal) > 10000000) {
		print(fd, "The bank manager sadly refuses your deposit.\n");
		print(fd, "That would make your balance larger than allowed.\n");
		return(PROMPT);
	}

	if(F_ISSET(ply_ptr, PLOANB)) {
		if(amt >= -ply_ptr->bank_bal) {
			ply_ptr->bank_bal = amt + ply_ptr->bank_bal;
			print(fd, "Thank you for repaying your loan.\n");
			if(ply_ptr->bank_bal < 100)
				print(fd, "Bank fees reduce your balance to $0.0\n");
			else
				print(fd, "Your bank balance is $%ld in gold.\n",
					ply_ptr->bank_bal);
			F_CLR(ply_ptr, PLOANB);
			F_CLR(ply_ptr, PLOANU);
			ply_ptr->gold -= amt;
		}
		else {
			print(fd, "You must repay your entire loan.\n");
			print(fd, "No bank deposits will be accepted for less.\n");
		}
		return(PROMPT);
	}

	fee = amt/50; 

	print(fd, "You deposit $%ld in gold.\n", amt);
	print(fd, "You pay %ld gold pieces in fees.\n", fee);

	ply_ptr->gold -= amt;
	ply_ptr->bank_bal += (amt - fee);
	if(ply_ptr->bank_bal <= 0)
		ply_ptr->bank_bal = 0;
	print(fd, "Your bank balance is $%ld in gold.\n", ply_ptr->bank_bal);

	savegame(ply_ptr, 0);
	return(PROMPT);
}

/*************************************************************************/
/*   			bank_transfer		  			 */
/*************************************************************************/

int bank_transfer(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	int 	fd = ply_ptr->fd;
	int 	not_logged = 0;
	long 	amt, xfee;
	creature *crt_ptr;		/* player receiving transfer */
	
	if(!F_ISSET(ply_ptr->parent_rom, RPBANK)) {
		print(fd, "This is not a bank window.\n");
		if(ply_ptr->class < CARETAKER)
		return(PROMPT);
	}

	if(ply_ptr->bank_bal < 1) {
		print(fd, "You don't have any gold to transfer.\n");
		return(PROMPT);
	}

	if(cmnd->num < 3) {
		print(fd, "Transfer how much, and to whom?\n");
		return(PROMPT);
	}

	if(cmnd->str[1][0] != '$') {
		print(fd, "The amount must be preceded by a $.\n");
		return(PROMPT);
	}

	amt = atol(&cmnd->str[1][1]);
	xfee = amt/20;

	if(amt < 1) {
		print(fd, "What are you trying to pull?  I should tell %s.\n",
			DMNAME);
		return(PROMPT);
	}

	if(amt < 100) {
		print(fd, "You can only transfer 100 gold pieces, or more.\n");
		return(PROMPT);
	}

	if(amt + xfee > ply_ptr->bank_bal) {
		print(fd, "You don't have that much gold.  The transfer fee is 5 percent.\n");
		return(PROMPT);
	}

	cmnd->str[2][0] = up(cmnd->str[2][0]);
    lowercize(cmnd->str[1], 1);
    crt_ptr = find_who(cmnd->str[2]);

	if(!crt_ptr) {
		if(load_ply(cmnd->str[2], &crt_ptr) < 0) {
			print(fd, "That person does not exist.\n"); 
			print(fd, "Is this some sort of bank scam?\n");
			return(PROMPT);
		}
		not_logged = 1;
	}

	if(!strcmp(crt_ptr->name, ply_ptr->name)) {
		print(fd, "Don't be silly.  That's your own account.\n");
		return(PROMPT);
	}

	if(amt + crt_ptr->bank_bal > 10000000) {
		print(fd, "Transfer declined.\n");
		print(fd, "That raises %s's balance over our limit.  Sorry.\n",
			crt_ptr);
		return(PROMPT);
	}

	crt_ptr->bank_bal += amt;
	if(crt_ptr->bank_bal >= 0) { 
		if(F_ISSET(crt_ptr, PLOANB))
			F_CLR(crt_ptr, PLOANB);	
	}
	ply_ptr->bank_bal -= (amt + xfee);
	savegame(ply_ptr, 0);
	savegame(crt_ptr, 0);

	print(fd, "Transfer fee of $%ld in gold pieces deducted.\n", xfee);
	print(fd, "%ld gold pieces transferred to %s.\n", amt, crt_ptr);
	print(fd, "Your new balance is $%ld in gold.\n", ply_ptr->bank_bal);

	if(not_logged)
		free(crt_ptr);

	return(PROMPT);
}

/*************************************************************************/
/*                            bank_loan                                  */
/*************************************************************************/
/*	This command allows a player to borrow from the bank.	         */

int bank_loan(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	long fee;
	long amt;
	int fd = ply_ptr->fd;

	if(!F_ISSET(ply_ptr->parent_rom, RPBANK)) {
		print(fd, "This is not a bank window.\n");
		if(ply_ptr->class < CARETAKER)
		return(PROMPT);
	}

	if(ply_ptr->level < 5) {
		print(fd, "The bank does not loan money to people at your level.\n");
		return(PROMPT);
	}

	if(cmnd->num < 2) {
		print(fd, "You must specify how much you wish to borrow.\n");
		print(fd, "Don't forget to use a $ sign with the amount.\n");
		return(PROMPT);
	}

	if(cmnd->str[1][0] != '$') {
		print(fd, "The amount must be preceded by a $.\n");
		return(PROMPT);
	}

	amt = atol(&cmnd->str[1][1]);
	if(amt < 1000) {
		print(fd, "Bank loan are between $1,000 and $100,000.\n");
		return(PROMPT);
	}

	if(amt > 10000 * ply_ptr->level) {
		print(fd, "We do not make loans that large to people of your level.\n");
		return(PROMPT);
	}
	

	if(ply_ptr->bank_bal > 0) {
		print(fd, "You must close your present account first.\n");
		print(fd, "Please withdraw all of you funds.\n");
		print(fd, "A new account will be opened with the loan.\n");
		return(PROMPT);
	}
	

	if(F_ISSET(ply_ptr, PLOANB)) {
		print(fd, 
			"The bank cannot make another loan until the first is repaid.\n");
		return(PROMPT);
	}

	fee = .1 * amt; 

	print(fd, "The bank loans you $%ld in gold.\n", (amt + fee));
	print(fd, "The fee will be $%ld in gold.\n", fee);
	ply_ptr->bank_bal += (amt + fee);
	ply_ptr->bank_bal = -ply_ptr->bank_bal;
	print(fd, "Your new account has a loan balance of $%ld in gold.\n", 
		(amt + fee)); 
	print(fd, "You may use up to $%ld for training.\n", amt);
	print(fd, "No more bank activity is allowed until this loan is repaid.\n");

	F_SET(ply_ptr, PLOANB);

	savegame(ply_ptr, 0);
	return(PROMPT);
}
