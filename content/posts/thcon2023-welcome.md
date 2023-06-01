---
layout: post
title: "Welcome (THCon CTF 2023)"
author: cryptax
date: 2023-04-22
tags:
- THCon
- CTF
- 2023
- Web3
- Ethereum
- Metamask
- remix
---


## Description

The challenge provides the following contract, and the goal is to "unlock" to get the flag.

```
pragma solidity ^0.7;

contract welcome {

    bool public isLocked;
    string public key;

    constructor(string memory _key) {
        isLocked = true;
        _key = key;
    }

    function unlock(string memory _something) public {
        if (keccak256(abi.encodePacked(key)) == keccak256(abi.encodePacked(_something))) {
            isLocked = false;
        }
    }

}
```

## Setup

The CTF provided a very nice tutorial to setup the THCONETH network:

1. Setup [Metamask](https://metamask.io/download/) and install it in the browser
2. Add access to the THCONETH network (created for THCon CTF). 2 networks were provided THConETH_1 and THConETH_2 as backup endpoint.
```
Network Name: THConETH_1
RPC URL: https://node01-eth.ctf.thcon.party:8545
Chain ID: 31400
Currency Symbol: THConETH
Block Explorer URL: (leave empty)
```
3. Connect to one of these networks ("Switch network" in Metamask). This gives us our personal login address.

Another important point in the tutorial was that, on the THConETH network, we were asked to set transactions with a minimum base fee of 110 GWEI, and a minimum priority fee of 55 GWEI.

![](/images/thcon23-web3-tuto.png)

## Deploy the contract

To start the challenge, we need 2 things:

1. To **deploy** the contract. Just need to click a button on the CTF infrastructure. Once deployed, the interface gives us the contract' address.
2. To **mine** a few THConETH tokens to be able to perform a few transactions. This could be done by mining on one of the 2 provided faucets (https://node0X-eth.ctf.thcon.party where X is 1 or 2)

![](/images/thcon23-welcome-faucet.png){ width=50% }

Specify your personal login address as recipient ETH address.

## Solving the challenge

In the contract, we see that to unlock, we need a **key**:

```
function unlock(string memory _something) public {
        if (keccak256(abi.encodePacked(key)) == keccak256(abi.encodePacked(_something))) {
            isLocked = false;
        }
    }
```

So, the idea is to read the key. 

I used an online tool: [remix.ethereum.org](http://remix.ethereum.org).
In "Deploy & Run transactions", select "At Address" and specify the address of the contract which was deployed.

![](/images/thcon23-remix-deploy.png)

Then click on the button "key" to read the value of the key.

![](/images/thcon23-welcome-key.png)

Notice the key is `WelcomeToTHCON23_WelcomeToSolidity`.

Then, we just need to call `unlock` with this key. This can be done with the Remix website: specify the key and click on Transact.

A pop-up will appear in Metamask. You need to ensure that the base and priority fees are set as specified in the tutorial for the transaction to be processed. Once processed, unlock succeeds and the challenge is unlocked. The challenge web page provides the flag :)

![](/images/thcon23-welcome-solved.png)

## My feedback

I had looked into a few Web3 challenges at Insomni'hack, so my first initiative for this one was to do everything from a Python script. I attempted to read the key with:

```
key = web3.to_text(web3.eth.get_storage_at(contract_address,1))
```

Then, I was hinted this was really overkill for this level, and started looking into online tools and ended on Remix. At first, everything is a bit mysterious, and I confess I'm not sure I fully understood everything (e.g are we able to read `key` because it is `public`?), but I certainly know more than at the beginning! This challenge was really cool to learn about block chains, and I loved it that the CTF created their own currency and block chain for us to have fun with. Thanks!
