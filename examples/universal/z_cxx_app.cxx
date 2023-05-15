//
// Copyright (c) 2023 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//
#include <iostream>

//
// zenoh.hxx automatically selects zenoh-c or zenoh-pico C++ wrapper
// depending on ZENOHCXX_ZENOHPICO or ZENOHCXX_ZENOHC setting
// and places it to the zenoh namespace
//
#include <iostream>

#include "zenoh.hxx"
using namespace zenoh;

class Publishers {
   public:
    Publishers() : session(nullptr) {}
    void set_session(std::shared_ptr<Session> s) { session = std::move(s); }
    void add_publisher() {
        std::string res_name = "demo/example/" + std::to_string(pubs.size());
        Publisher p = std::get<Publisher>(session->declare_publisher(res_name));
        pubs.push_back(std::move(p));
    }
    void pub(const std::string_view& value) {
        std::cout << "sending  '" << value << "' to " << pubs.size() << " publishers\n";
        for (auto& p : pubs) {
            p.put(value);
        }
    }

   private:
    std::shared_ptr<Session> session;
    std::vector<Publisher> pubs;
};

class Subscribers {
   public:
    Subscribers() : session(nullptr) {}
    void set_session(std::shared_ptr<Session> s) { session = std::move(s); }
    void add_subscriber() {
        std::string res_name = "demo/example/*";
        auto n = subs.size();
        Subscriber s = std::get<Subscriber>(session->declare_subscriber(res_name, [this, n](const Sample* sample) {
            if (sample) {
                this->on_receive(n, sample->get_keyexpr().as_string_view(), sample->get_payload().as_string_view());
            }
        }));
        subs.push_back(std::move(s));
    }
    void on_receive(size_t n, const std::string_view& keyexpr, const std::string_view& value) {
        std::cout << "received : '" << value << "' from " << keyexpr << " on receiver " << n << "\n";
    }

   private:
    std::shared_ptr<Session> session;
    std::vector<Subscriber> subs;
};

class CustomerClass {
   public:
    // Class is not allowed to be moved or copied
    // These restrinctions are to be removed after integration of https://github.com/eclipse-zenoh/zenoh-c/pull/157
    CustomerClass(CustomerClass&&) = delete;
    CustomerClass(const CustomerClass&) = delete;
    CustomerClass& operator=(const CustomerClass&) = delete;
    CustomerClass& operator=(CustomerClass&&) = delete;

    CustomerClass() : session(nullptr) {
        Config config;
        Session s = std::get<Session>(open(std::move(config)));
        session = std::make_shared<Session>(std::move(s));
        pubs.set_session(session);
        subs.set_session(session);
    }

    void add_publishers(int n) {
        for (int i = 0; i < n; i++) {
            pubs.add_publisher();
        }
    }
    void add_subscribers(int n) {
        for (int i = 0; i < n; i++) {
            subs.add_subscriber();
        }
    }
    void pub(const std::string_view& value) { pubs.pub(value); }

   private:
    std::shared_ptr<Session> session;
    Publishers pubs;
    Subscribers subs;
};

int _main(int argc, char** argv) {
    CustomerClass customer;
    customer.add_publishers(3);
    customer.add_subscribers(5);
    customer.pub("Hello World");
    return 0;
}

int main(int argc, char** argv) {
    try {
        return _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
