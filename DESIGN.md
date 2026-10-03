# M2 Technical & Design Understanding

This file is an assessed technical-understanding artifact, not ordinary project documentation.
Answer all four questions using your own submitted implementation. Concise answers are acceptable when they are technically correct and specific.

Generic descriptions of C++ concepts or restatements of the assignment that do not identify and explain corresponding parts of your code will receive limited credit.

## 1. Polymorphism and dynamic dispatch - 1.5 points

Identify one place in your M2 implementation where runtime polymorphism occurs. Name the relevant base interface, derived implementation, and `ProcessingCore` function involved. Trace the call from `ProcessingCore` to the selected strategy implementation and explain why the derived implementation is invoked.

Then explain what would change if the relevant operation were not declared `virtual`.

Runtime polyorphism is implememted in ProcessingCore::search(), which relies on RetrievalStrategy's base interface. Initally, the derived implementation is RetrievalEngine, however custom implementations like DummyRetrievalStrategy cna be injected. When the client calls ProcessingCore::search(query, k), the core executes impl_->retrieval->search(...). Since impl_->retrieval is a base class pointer, the C++ runtime inspects the actual object it points to and dynamically calls to the overriden search() method of the specific class in memory. If search() was not declared virtual in RetrievalStrategy, C++ would use static binding. The complier would resolve the function call based on the pointer's type and ignore the derived class. This would bring on the base class default implementation.
## 2. Ownership and lifetime - 1.5 points

Identify where one of the strategy objects is created, where ownership is transferred, and which object ultimately owns it. Explain how `std::unique_ptr` represents that ownership relationship and when the strategy object is destroyed.

Also explain why `ProcessingCore` is move-only and why the strategy base classes require virtual destructors.

Strategy objects are created by the caller either in the ProcessingCore or in testing. Ownership is then transferred into ProcessingCore through its constructor via std::move(), then the objects are finally owned by the ProcessingCore::Impl struct. std::unique_ptr enforces this ownership by deleting the copy constructor, this physically prevents multiple parts of the system from claiming ownership of the same strategy instance. The strategy onjects are destroyed automatically through RAII when ProcessingCore::impl goes out of scope and is destroyed. ProcessingCore is move only since it manages std::unique_ptr. Attempting to copy the core would require copying unique pointers which is illegal. The strategy base classes require virtual destructors since ProcessingCore deletes them through base class pointers. With no virtual destructor, only the base class destructors would execute and cause memory leaks.

## 3. Architecture, extensibility, and M1 compatibility - 1.5 points

Explain one specific architectural decision in your M2 implementation that makes the processing system extensible while preserving M1 behavior.

Identify the classes or interfaces involved and explain both:
- how the default configuration preserves M1 behavior; and
- how a different implementation can be substituted without changing the normal `ProcessingCore` API.

Include one plausible design alternative and explain why the M2 design is preferable for this milestone. The alternative does not need to be something you actually implemented.

The primary architecture design is combining constructor dependency injection woth the strategy pattern. To preserve M1 behavior, the default ProcessingCore contructor implements the M1 classes and stores them inside the Impl strct's std::unique_ptr strategy interfaces. To substitute a differnet implementation, a caller implements a derived class and passes it to paramaterized ProcessingCore. Since the core's public API only interacts with the base interfaces, the logic changes seamlessly without messing with the core method signatures. A dessign alternative would be a factory registry, where strategies are requested by passing string names. I prefer the constructor injection design since it relies on compile time safety and ownership.

## 4. Testing and defect reasoning - 1.5 points

Select one meaningful test from your `tests/student_tests.cpp`.

Explain:
- what M2 requirement the test validates;
- what specific implementation defect the test could detect; and
- why your test provides useful evidence beyond simply rerunning the supplied public tests.

If your test uses a custom strategy, explain how its observable behavior demonstrates that `ProcessingCore` is actually using runtime substitution.

A meaningful test is the Custom Strategies and Move Semantics test, this test validates the M2 requirement that move operations leave ownership safe and that runtime substitiution operates sucessfully. It specifically prevents defects where move constructor could shallow copy state or fail to transfer the dynamically allocated polymorphic pointers properly, which could cause a crash or a fallback to default behavior. This provides a useful batch of evidence beyond the public tests since it verifies that the custom polymorphic objects survive C++ move semantics. The test proves that ProcessingCore is using runtime substitution through its behavior. The injected GiantChunkStrategy is programmed to output chunks with a token_count of 999, since the defauly M1 chunker caps tokens at 120, so asserting token_count at 999 guarentees that the custom strategy was deployed.
