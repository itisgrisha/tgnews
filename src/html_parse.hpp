#pragma once

#include "lexbor/html/parser.h"
#include "lexbor/dom/interfaces/element.h"

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>


class LXBCollection {
public:
    LXBCollection(lxb_dom_document_t* document,
                  const std::string& tag_name) {
        collection_ = lxb_dom_collection_make(document, 50);
        lxb_dom_elements_by_tag_name(
            lxb_dom_interface_element(document),
            collection_,
            reinterpret_cast<const lxb_char_t*>(tag_name.c_str()),
            tag_name.size());
    }

    size_t Size() const {
        return lxb_dom_collection_length(collection_);
    }

    lxb_dom_element_t* operator[](size_t i) const {
        return lxb_dom_collection_element(collection_, i);
    }

    ~LXBCollection() {
        lxb_dom_collection_destroy(collection_, false);
    }

private:
    lxb_dom_collection_t* collection_;
};


class HTMLDocument {
public:
    explicit HTMLDocument(const std::string& file_path) {
        document_ = lxb_html_document_create();
        ReadDocument(file_path);
        ParseDocument();
    }

    std::string GetText() const {
        return text_;
    }

    const char* GetTextP() const {
        return text_.c_str();
    }

    const std::vector<std::string> GetMetaKeys() const {
        std::vector<std::string> keys;
        for (const auto& pair : meta_) {
            keys.emplace_back(pair.first);
        }
        return keys;
    }

    std::string GetMeta(const std::string& key) const {
        auto it = meta_.find(key);
        if (it == meta_.end()) {
            return {""};
        }
        return it->second;
    }

    void SetMeta(const std::string& key, const std::string& value) {
        meta_[key] = value;
    }

    const std::vector<std::string> GetLinks() const {
        return related_links_;
    }

    HTMLDocument(const HTMLDocument& other) = delete;
    HTMLDocument& operator=(const HTMLDocument& other) = delete;

    HTMLDocument(HTMLDocument&& other) {
        text_ = std::move(other.text_);
        meta_ = std::move(other.meta_);
        related_links_ = std::move(other.related_links_);

        html_ = other.html_;
        document_ = other.document_;
        html_size_ = other.html_size_;
        other.html_ = nullptr;
        other.document_ = nullptr;
        other.html_size_ = 0;
    }

    ~HTMLDocument() {
        if (document_ != nullptr) {
            delete[] html_;
            lxb_html_document_destroy(document_);
        }
    }

private:
    lxb_html_document_t* document_;
    lxb_char_t* html_;
    size_t html_size_;

    std::unordered_map<std::string, std::string> meta_;
    std::string text_;
    std::vector<std::string> related_links_;

    void ReadDocument(const std::string& path) {
        std::ifstream input_fstream;
        input_fstream.open(path);
        auto file_buf = input_fstream.rdbuf();
        html_size_ = file_buf->pubseekoff(0, input_fstream.end, input_fstream.in);
        file_buf->pubseekpos(0, input_fstream.in);
        html_ = new lxb_char_t[html_size_];
        file_buf->sgetn(reinterpret_cast<char*>(html_), html_size_);
        input_fstream.close();
    }

    std::string GetRawAttribute(lxb_dom_element_t* node, const std::string& attribute_name) {
        size_t value_len;
        const lxb_char_t* attr = reinterpret_cast<const lxb_char_t*>(attribute_name.c_str());
        const lxb_char_t* value = lxb_dom_element_get_attribute(
                node, attr, attribute_name.size(), &value_len);
        return {reinterpret_cast<const char*>(value), value_len};
    }

    void ParseMeta() {
        LXBCollection collection(lxb_dom_interface_document(document_), "meta");
        for (size_t i = 0; i < collection.Size(); ++i) {
            auto element = collection[i];
            auto property = GetRawAttribute(element, "property");
            if (property.size() > 0) {
                meta_[property] = GetRawAttribute(element, "content");
            }
        }
    }

    void ParseText() {
        LXBCollection collection(lxb_dom_interface_document(document_), "p");
        text_ = "";
        for (size_t i = 0; i < collection.Size(); ++i) {
            auto element_ = collection[i];
            size_t text_size;
            auto p_lxb_text =
                lxb_dom_node_text_content(lxb_dom_interface_node(element_), &text_size);
            text_ += std::string(reinterpret_cast<char*>(p_lxb_text), text_size);
        }
    }

    void ParseLinks() {
        LXBCollection collection(lxb_dom_interface_document(document_), "related");
        if (collection.Size() > 0) {
            auto collection_a = lxb_dom_collection_make(lxb_dom_interface_document(document_), 20);
            lxb_dom_elements_by_tag_name(
                lxb_dom_interface_element(document_),
                collection_a,
                reinterpret_cast<const lxb_char_t*>("a"),
                1);
            for (size_t i = 0; i < lxb_dom_collection_length(collection_a); ++i) {
                auto element = lxb_dom_collection_element(collection_a, i);
                auto href = GetRawAttribute(element, "href");
                if (href.size() > 0) {
                    related_links_.emplace_back(href);
                }
            }
            lxb_dom_collection_destroy(collection_a, false);
        }
    }

    void ParseDocument() {
         lxb_html_document_parse(document_, html_, html_size_);
         ParseMeta();
         ParseText();
         ParseLinks();
    }
};
