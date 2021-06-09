# Address Book Local Service Module


The Alexa Auto SDK Address Book Local Service module provides user data such as contacts and navigation favorites ("home", "work", etc.) to services running as part of Local Voice Control. Local skills like Comms and Navigation can then use this data to call a contact or navigate to a favorite address, for example. This module is used in conjunction with the [Local Skill Service](../../modules/local-skill-service/README.md) module.

Address Book Local Service uses the [Address Book](../../../../../modules/address-book/README.md) platform interfaces to retrieve address book entries and perform other address book life cycle operations. For more details on the Address Book platform interface, please see the [Address Book README](../../../../../modules/address-book/README.md).