#include "NVM_Transaction_Flash.h"


namespace SSD_Components
{
	NVM_Transaction_Flash::NVM_Transaction_Flash(Transaction_Source_Type source, Transaction_Type type, stream_id_type stream_id,
		unsigned int data_size_in_byte, LPA_type lpa, PPA_type ppa, User_Request* user_request) :
		NVM_Transaction(stream_id, source, type, user_request),
#ifdef NEW_SUBMIT
		Data_and_metadata_size_in_byte(data_size_in_byte), LPA(lpa), PPA(ppa), Physical_address_determined(false), FLIN_Barrier(false), isSubblock_Erase_Tr(false)
#else
		Data_and_metadata_size_in_byte(data_size_in_byte), LPA(lpa), PPA(ppa), Physical_address_determined(false), FLIN_Barrier(false)
#endif
	{
	}
	
	NVM_Transaction_Flash::NVM_Transaction_Flash(Transaction_Source_Type source, Transaction_Type type, stream_id_type stream_id,
		unsigned int data_size_in_byte, LPA_type lpa, PPA_type ppa, const NVM::FlashMemory::Physical_Page_Address& address, User_Request* user_request) :
		NVM_Transaction(stream_id, source, type, user_request), Data_and_metadata_size_in_byte(data_size_in_byte), LPA(lpa), PPA(ppa), Address(address), Physical_address_determined(false)

	{}
}
