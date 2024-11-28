#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <bitset>
#include <string>

using namespace std;

class HuffmanCoding {
private:
    class HeapNode {
    public:
        char character;
        int frequency;
        HeapNode* left;
        HeapNode* right;

        HeapNode(char ch, int freq) : character(ch), frequency(freq), left(nullptr), right(nullptr) {}

        bool operator<(const HeapNode& other) const {
            return frequency > other.frequency;
        }
    };

    unordered_map<char, string> codes;
    unordered_map<string, char> reverse_mapping;
    priority_queue<HeapNode> heap;

    void makeFrequencyDict(const string& text, unordered_map<char, int>& frequency) {
        for (char ch : text) {
            frequency[ch]++;
        }
    }

    void makeHeap(const unordered_map<char, int>& frequency) {
        for (auto pair : frequency) {
            heap.push(HeapNode(pair.first, pair.second));
        }
    }

    void mergeNodes() {
        while (heap.size() > 1) {
            HeapNode left = heap.top();
            heap.pop();
            HeapNode right = heap.top();
            heap.pop();

            HeapNode merged('\0', left.frequency + right.frequency);
            merged.left = new HeapNode(left);
            merged.right = new HeapNode(right);

            heap.push(merged);
        }
    }

    void makeCodesHelper(HeapNode* root, const string& currentCode) {
        if (!root) return;

        if (root->character != '\0') {
            codes[root->character] = currentCode;
            reverse_mapping[currentCode] = root->character;
            return;
        }

        makeCodesHelper(root->left, currentCode + "0");
        makeCodesHelper(root->right, currentCode + "1");
    }

    void makeCodes() {
        HeapNode* root = new HeapNode(heap.top());
        heap.pop();
        makeCodesHelper(root, "");
    }

    string getEncodedText(const string& text) {
        string encodedText = "";
        for (char ch : text) {
            encodedText += codes[ch];
        }
        return encodedText;
    }

    string padEncodedText(const string& encodedText) {
        int extraPadding = 8 - encodedText.size() % 8;
        string paddedText = encodedText + string(extraPadding, '0');

        string paddingInfo = bitset<8>(extraPadding).to_string();
        return paddingInfo + paddedText;
    }

    vector<uint8_t> getByteArray(const string& paddedEncodedText) {
        vector<uint8_t> byteArray;

        for (size_t i = 0; i < paddedEncodedText.size(); i += 8) {
            bitset<8> byte(paddedEncodedText.substr(i, 8));
            byteArray.push_back(static_cast<uint8_t>(byte.to_ulong()));
        }

        return byteArray;
    }

    string removePadding(const string& paddedEncodedText) {
        string paddingInfo = paddedEncodedText.substr(0, 8);
        int extraPadding = bitset<8>(paddingInfo).to_ulong();

        string encodedText = paddedEncodedText.substr(8);
        return encodedText.substr(0, encodedText.size() - extraPadding);
    }

    string decodeText(const string& encodedText) {
        string currentCode = "";
        string decodedText = "";

        for (char bit : encodedText) {
            currentCode += bit;
            if (reverse_mapping.find(currentCode) != reverse_mapping.end()) {
                decodedText += reverse_mapping[currentCode];
                currentCode = "";
            }
        }

        return decodedText;
    }

public:
    HuffmanCoding() = default;

    string compress(const string& inputPath) {
        ifstream file(inputPath, ios::binary);
        if (!file) {
            cerr << "Error opening input file: " << inputPath << endl;
            return "";
        }

        string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        unordered_map<char, int> frequency;
        makeFrequencyDict(text, frequency);
        makeHeap(frequency);
        mergeNodes();
        makeCodes();

        string encodedText = getEncodedText(text);
        string paddedEncodedText = padEncodedText(encodedText);
        vector<uint8_t> byteArray = getByteArray(paddedEncodedText);

        string outputPath = inputPath + ".bin";
        ofstream output(outputPath, ios::binary);
        if (!output) {
            cerr << "Error opening output file: " << outputPath << endl;
            return "";
        }

        output.write(reinterpret_cast<const char*>(byteArray.data()), byteArray.size());
        output.close();

        cout << "Compressed" << endl;
        return outputPath;
    }

    string decompress(const string& inputPath) {
        ifstream file(inputPath, ios::binary);
        if (!file) {
            cerr << "Error opening input file: " << inputPath << endl;
            return "";
        }

        string bitString = "";
        uint8_t byte;
        while (file.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
            bitString += bitset<8>(byte).to_string();
        }
        file.close();

        string encodedText = removePadding(bitString);
        string decompressedText = decodeText(encodedText);

        string outputPath = inputPath + "_decompressed.txt";
        ofstream output(outputPath);
        if (!output) {
            cerr << "Error opening output file: " << outputPath << endl;
            return "";
        }

        output << decompressedText;
        output.close();

        cout << "Decompressed" << endl;
        return outputPath;
    }
};

int main() {
    HuffmanCoding huffman;
    string inputFilePath = "input.txt";

    string compressedFilePath = huffman.compress(inputFilePath);
    if (!compressedFilePath.empty()) {
        string decompressedFilePath = huffman.decompress(compressedFilePath);
    }

    return 0;
}
