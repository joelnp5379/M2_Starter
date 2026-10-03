#include "aiws/processing_core.hpp"
#include "aiws/chunking_strategy.hpp"
#include "aiws/retrieval_strategy.hpp"
#include "aiws/context_strategy.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int failures = 0;
void check(bool ok, const char* name) {
    if (!ok) { std::cerr << "FAIL: " << name << '\n'; ++failures; }
}

// Student-written M2 tests
//
// Add your own tests to this file. Your tests are part of the submitted work
// and are evaluated under Student-Written Testing & Validation.
//
// Do not modify tests/public_tests.cpp.
//
// Your tests should exercise important M2 behavior beyond the supplied public
// tests. Consider default compatibility, custom strategies, runtime dispatch,
// invalid configuration, ownership/move behavior, and component interactions.

// Always creates one chunk with an exaggerated token count
class GiantChunkStrategy final : public aiws::ChunkingStrategy {
public:
    std::vector<aiws::Chunk> chunk(const aiws::Document& d, std::size_t order) const override {
        return {{d.id() + "#giant", d.id(), order, 0, d.text(), 999, 0, d.text().size()}};
    }
};
// Always returns a single dummy search result
class DummyRetrievalStrategy final : public aiws::RetrievalStrategy {
public:
    std::vector<aiws::SearchResult> search(const std::string&, int,
        const std::vector<aiws::Chunk>&, const aiws::CorpusIndex&) const override {
        return {{"dummy#0", "dummy", 0, "dummy text", 100.0, 5}};
    }
};
// Ignores the budget and returns a bypassed flag
class BypassContextStrategy final : public aiws::ContextStrategy {
public:
    std::vector<aiws::ContextItem> build(const std::vector<aiws::SearchResult>& ranked,
                                         std::size_t) const override {
        if (ranked.empty()) return {};
        return {{ranked[0].chunk_id, ranked[0].document_id, ranked[0].chunk_sequence, "bypassed", 99, 100.0, false}};
    }
};
}

int main() {
    using namespace aiws;

    // Test 1: Duplicate ID Rebuild Protection
    Workspace valid_ws;
    valid_ws.add_document(Document{"doc1", "T1", "Text 1"});
    valid_ws.add_document(Document{"doc2", "T2", "Text 2"});
    
    ProcessingCore core;
    core.rebuild(valid_ws);
    std::size_t original_count = core.chunk_count();
    check(original_count > 0, "Default rebuild populates chunks");

    Workspace invalid_ws;
    invalid_ws.add_document(Document{"doc3", "T3", "Text 3"});
    invalid_ws.add_document(Document{"doc3", "T4", "Duplicate ID"}); // Duplicate invalidates rebuild
    
    bool threw_dup = false;
    try {
        core.rebuild(invalid_ws);
    } catch (const std::invalid_argument&) {
        threw_dup = true;
    }
    check(threw_dup, "Rebuild with duplicate IDs throws invalid_argument");
    check(core.chunk_count() == original_count, "Failed rebuild preserves previous valid corpus");

    // Test 2: Custom Strategies & Move Semantics
    ProcessingCore custom_core(std::make_unique<GiantChunkStrategy>(),
                               std::make_unique<DummyRetrievalStrategy>(),
                               std::make_unique<BypassContextStrategy>());
    
    custom_core.rebuild(valid_ws);
    check(custom_core.chunks()[0].token_count == 999, "Custom chunker dynamically dispatched");

    ProcessingCore moved_core = std::move(custom_core);

    auto results = moved_core.search("test", 1);
    check(results.size() == 1 && results[0].document_id == "dummy", "Moved core retains custom retrieval strategy");
    
    auto ctx = moved_core.build_context("test", 1, 1);
    check(ctx.size() == 1 && ctx[0].text == "bypassed", "Moved core retains custom context strategy");

    // Test 3: Null Configuration Rejection
    bool threw_null = false;
    try {
        ProcessingCore null_core(std::make_unique<GiantChunkStrategy>(),
                                 nullptr, // Invalid null strategy
                                 std::make_unique<BypassContextStrategy>());
    } catch (const std::invalid_argument&) {
        threw_null = true;
    }
    check(threw_null, "Constructor with null strategy throws invalid_argument");

    if (failures == 0) {
        std::cout << "All student tests passed.\n";
        return 0;
    }
    std::cerr << failures << " student test(s) failed.\n";
    return 1;
}
