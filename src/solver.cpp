#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <map>
#include <queue>
#include <unordered_map>
using namespace std;

int MAX_WORD_SIZE = 13;
int MIN_WORD_SIZE = 3;

void fail(const string &msg) {
    cerr << msg << endl;
    exit(43);
}

void panic(const string &msg) {
    fail("Panika: " + msg + ". Ozvěte se organizátorům.");
}

int calc_freespace(int x, int y, int dx, int dy, const vector<char> &generated_crossword, int matrix_size) {
    int free_space = 0;
    while (x >= 0 && x < matrix_size && y >= 0 && y < matrix_size && generated_crossword[x * matrix_size + y] == '#') {
        free_space++;
        x += dx;
        y += dy;
        if (free_space > MAX_WORD_SIZE) {
            return free_space;
        }
    }
    return free_space;
}
struct Position{
    pair<int,int> start;
    int dx;
    int dy;
    Position(){}
    Position(pair<int,int> start,int dx,int dy):start(start),dx(dx),dy(dy){}
};
class Aho {
    struct Vertex {
        std::unordered_map<char, int> children {}; 
        int parent = -1; 
        int suffix_link = -1; 
        int end_word_link = -1; 
        int word_ID = -1;
        char parent_char;
        bool leaf = false;
    };
    std::vector<Vertex> trie;
    std::vector<int> words_length;
    int size;
    int root;
    int wordID;
    
public:
	map<string,int>duplicate_words;
    vector<pair<pair<int,int>,char>> placed_chars;
	map<string,vector<Position>> placed_words;
    
    Aho(){
        size = 1;
        root = 0;
        wordID = 0;
        trie.push_back(Vertex{});
    }
    
    void add_string(const std::string& s){
        int curVertex = root;
        for (char c : s){
            if (trie[curVertex].children.find(c) == trie[curVertex].children.end()){
                trie.push_back(Vertex{});
                trie[size].parent = curVertex;
                trie[size].parent_char = c;
                trie[curVertex].children[c] = size;
                size++;
            }
            curVertex = trie[curVertex].children[c];
        }

        trie[curVertex].leaf = true;
        trie[curVertex].word_ID = wordID;
        words_length.push_back(s.size());
        ++wordID;
    }
    
    void prepare(){
        std::queue<int> vertexQueue {};
        vertexQueue.push(root);
        while (!vertexQueue.empty()){
            int curVertex = vertexQueue.front();
            vertexQueue.pop();
            CalcSuffLink(curVertex);
            for (const auto& p : trie[curVertex].children)
                vertexQueue.push(trie[curVertex].children[p.first]);
        }
    }
    bool deleting_wrong_spot(int x,int y){
        for(auto character : placed_chars){
            pair<int,int> coord = character.first;
            if(coord.first==x && coord.second==y){
                return true;
            }
        }
        return false;
    }
    bool find_cords_in_placed_words(vector<Position> positions, int x, int y){
        for(auto pos:positions){
            if(pos.start.first==x && pos.start.second==y){
                return false;
            }
        }
        return true;
        
    }
    int process(const std::string& text,int i,int matrix_size,bool diagonal=false,bool vertical=false){
        int currentState = root;
        int result = 0;
        for (int j = 0; j < text.size(); j++){
            while (true){
                if (trie[currentState].children.find(text[j]) != trie[currentState].children.end()){
                    currentState = trie[currentState].children[text[j]];
                    break;
                }
                if (currentState == root) break;
                
                currentState = trie[currentState].suffix_link;
            }
            int checkState = currentState;
            while (true){ 
                checkState = trie[checkState].end_word_link;

                if (checkState == root) break;
                result++;
                int indexOfMatch = j + 1 - words_length[trie[checkState].word_ID];

                string matched =text.substr(indexOfMatch, words_length[trie[checkState].word_ID]);
                //std::cerr << "There was a match at index " << indexOfMatch << ": ";
                //std::cerr << matched << " " << i << " " << diagonal << " " << vertical << std::endl;
                
                if(placed_words.find(matched)==placed_words.end())
                    reverse(matched.begin(),matched.end());
				int ind_x, ind_y;
                int ind_x_last, ind_y_last;
                if (diagonal && !vertical) {
                    ind_x = indexOfMatch, ind_y =i + indexOfMatch;
                    ind_x_last =  indexOfMatch + matched.size() - 1, ind_y_last = i +indexOfMatch + matched.size() - 1;
                    if(ind_y<0){
                        ind_y=abs(matrix_size+i-indexOfMatch)-1;
                        ind_y_last=ind_y-matched.size();
                    }
                } else if (diagonal && vertical) {
                    ind_x = indexOfMatch, ind_y = indexOfMatch+i;
                    ind_x_last = indexOfMatch + matched.size() - 1, ind_y_last = indexOfMatch+i + matched.size() - 1;
                    if(ind_y<0){
                        ind_y=abs(i-indexOfMatch);
                        ind_y_last=ind_y+matched.size()-1;
                    }
                    
                } else if (vertical && !diagonal) {
					ind_x = indexOfMatch, ind_y = i;
                    ind_x_last = indexOfMatch+matched.size()-1, ind_y_last = i;
                }else{
					ind_x = i, ind_y = indexOfMatch;
                    ind_x_last = i, ind_y_last = indexOfMatch+matched.size()-1;
                }
                
                if (find_cords_in_placed_words(placed_words[matched], ind_x, ind_y)&& find_cords_in_placed_words(placed_words[matched], ind_y, ind_x)) {
                    
                    for (auto cord : placed_chars) {
                        if(vertical && !diagonal){
                            if (cord.first.first >= ind_x && cord.first.second == ind_y&&cord.first.first<=ind_x_last&&cord.first.second==ind_y_last) {
                                duplicate_words[matched]++;
                            }
                        }
                        if(!diagonal && !vertical){
                            if (cord.first.first == ind_x && cord.first.second >= ind_y && cord.first.first == ind_x_last && cord.first.second <= ind_y_last) {
                                duplicate_words[matched]++;
                            }
                        }
                        if(diagonal){
                            
                            if (cord.first.first >= ind_y && cord.first.second >= ind_x&&cord.first.first<=ind_y_last&&cord.first.second<=ind_x_last) {
                                duplicate_words[matched]++;
                            }else if (cord.first.first >= ind_x && cord.first.second >= ind_y&&cord.first.first<=ind_x_last&&cord.first.second<=ind_y_last) {
                                duplicate_words[matched]++;
                            }else if(cord.first.first >= ind_x && cord.first.second <= ind_y&&cord.first.first<=ind_x_last&&cord.first.second>=ind_y_last) {
                                duplicate_words[matched]++;
                            }else if(cord.first.second <= ind_x && cord.first.first >= ind_y&&cord.first.second>=ind_x_last&&cord.first.first<=ind_y_last) {
                                duplicate_words[matched]++;
                            }
                            
                            int ind_x_2=matrix_size-ind_x-1;
                            int ind_x_last_2=ind_x_2+matched.size()-1;
                            if(cord.first.second >= ind_x_2 && cord.first.first >= ind_y&&cord.first.second<=ind_x_last_2&&cord.first.first<=ind_y_last) {
                                duplicate_words[matched]++;
                            }
                            if(vertical){
                                int ind_y_2=abs(i-indexOfMatch);
                                int ind_y_last_2=ind_y_2+matched.size()-1;
                                if(cord.first.first >= ind_x && cord.first.second >= ind_y_2&&cord.first.first<=ind_x_last&&cord.first.second<=ind_y_last_2) {
                                    duplicate_words[matched]++;
                                }
                                if(cord.first.first <= ind_x && cord.first.second >= ind_y_2&&cord.first.first>=ind_x_last&&cord.first.second<=ind_y_last_2) {
                                    duplicate_words[matched]++;
                                }
    
                            }
                            if(!vertical){
                                int ind_y_2=abs(i+indexOfMatch);
                                int ind_y_last_2=ind_y_2-matched.size()-1;
                                if(cord.first.first >= ind_x && cord.first.second <= ind_y_2&&cord.first.first<=ind_x_last&&cord.first.second>=ind_y_last_2) {
                                    duplicate_words[matched]++;
                                }
                                if(cord.first.first <= ind_x && cord.first.second >= ind_y_2&&cord.first.first>=ind_x_last&&cord.first.second<=ind_y_last_2) {
                                    duplicate_words[matched]++;
                                }
                                ind_y_2=matrix_size+i-indexOfMatch-1;
                                ind_y_last_2=ind_y_2+matched.size();
                                if(cord.first.first >= ind_x && cord.first.second >= ind_y_2&&cord.first.first>=ind_x_last&&cord.first.second>=ind_y_last_2) {
                                    duplicate_words[matched]++;
                                }

                            }

                        }

                    }
                }
                checkState = trie[checkState].suffix_link;
            }
        }

        return result;
    }
    string get_closest_word(string text){
        // aho will receive a string and it should return the word that that has prefix as a suffix of our input string text and has the smallest length
        // if we didn't find any word that has prefix as a suffix of our input string text we should return "#"
        int currentState = root;
        for (int j = 0; j < text.size(); j++){
            while (true){
                if (trie[currentState].children.find(text[j]) != trie[currentState].children.end()){
                    currentState = trie[currentState].children[text[j]];
                    break;
                }
                if (currentState == root) break;
                
                currentState = trie[currentState].suffix_link;
            }
        }
        int checkState = currentState;
        while (true){ 
            checkState = trie[checkState].end_word_link;

            if (checkState == root) break;
            int indexOfMatch = text.size() - words_length[trie[checkState].word_ID];
            string matched =text.substr(indexOfMatch, words_length[trie[checkState].word_ID]);
            return matched;
        }
        return "#";
    }
    
    
    void CalcSuffLink(int vertex){
        if (vertex == root){ 
            trie[vertex].suffix_link = root;
            trie[vertex].end_word_link = root;
            return;
        }

        if (trie[vertex].parent == root){ 
            trie[vertex].suffix_link = root;
            
            if (trie[vertex].leaf) trie[vertex].end_word_link = vertex;
            else trie[vertex].end_word_link = trie[trie[vertex].suffix_link].end_word_link;
            
            return;
        }

        int curBetterVertex = trie[trie[vertex].parent].suffix_link;
        char chVertex = trie[vertex].parent_char; 

        while (true){

            if (trie[curBetterVertex].children.find(chVertex) != trie[curBetterVertex].children.end()){
                trie[vertex].suffix_link = trie[curBetterVertex].children[chVertex];
                break;
            }

            if (curBetterVertex == root){ 
                trie[vertex].suffix_link = root;
                break;
            }
            
            curBetterVertex = trie[curBetterVertex].suffix_link; // Go back by sufflink
        }
        if (trie[vertex].leaf){
            trie[vertex].end_word_link = vertex;
        } 
        else{
            trie[vertex].end_word_link = trie[trie[vertex].suffix_link].end_word_link;
        }
    }

}; 

void place_word(const string &word, int x, int y, int dx, int dy, vector<char> &generated_crossword, int matrix_size) {
    for (size_t i = 0; i < word.size(); i++) {
        generated_crossword[x * matrix_size + y] = word[i];
        x += dx;
        y += dy;
    }
}
pair<int,int> get_word_dir(const string &word, int x, int y, vector<char> &generated_crossword, int matrix_size) {
    vector<pair<int, int>> directions = {{0, 1}, {1, 0}, {1, 1}, {-1, -1}};
    for (auto dir : directions) {
        int count = 0;
        for(int i=0;i<word.size();i++){
            if(generated_crossword[(x+i*dir.first) * matrix_size + y+i*dir.second] == word[i]){
                count++;
            }
        }
        if(count==word.size()){
            return {dir.first,dir.second};
        }
    }
    return {0, 0};
}
bool q_placed =false;
bool w_placed =false;
bool x_placed =false;
bool hash_placed =false;

void remove_word(const string &word,Position pos, vector<char> &generated_crossword, int matrix_size,Aho &ahotrie) {
    int dx = pos.dx;
    int dy = pos.dy;
    int x = pos.start.first;
    int y = pos.start.second;
    if(dx==0 && dy==0){
        return;
    }
    for (size_t i = 0; i < word.size(); i++) {
        string charstoinsert="qwx#";
        
        generated_crossword[x * matrix_size + y] = charstoinsert[rand()%3];
        if(generated_crossword[x * matrix_size + y] == 'q'){
            q_placed = true;
        }
        if(generated_crossword[x * matrix_size + y] == 'w'){
            w_placed = true;
        }
        if(generated_crossword[x * matrix_size + y] == 'x'){
            x_placed = true;
        }
        if(generated_crossword[x * matrix_size + y] == '#'){
            hash_placed = true;
        }

        x += dx;
        y += dy;
    }
}
vector<int> calc_heuristic(const vector<string> &dictionar){//get indexes where word of size i starts in dictionar
	vector<int>word_heuristic(dictionar.back().size()+1);
    for(int i=dictionar.size();i>=0;i--){
        word_heuristic[dictionar[i].size()]=i;
    }
	return word_heuristic;
}
string find_word(const vector<string> &dictionar, const vector<int> &word_heuristic, int wordsize) {
    // dictionary is sorted by size so we can do binary search based on heurictic array
    int left = word_heuristic[wordsize];
    int right = wordsize+1;

    if(dictionar[dictionar.size()-1].size()==wordsize+1){
        right=dictionar.size()-1;
    }
    right=word_heuristic[right];
    if (left == right) {
        return "#";
    }
    return dictionar[rand() % (right - left) + left];
}
void check_ahotrie(Aho &ahotrie,vector<char>&generated_crossword,int matrix_size){
    vector<string> grid(matrix_size);
    vector<string> grid2(matrix_size);
    for (int i = 0; i < matrix_size; i++) {
        for (int k = 0; k < matrix_size; k++) {
            grid[i].push_back(generated_crossword[i * matrix_size + k]);
            grid2[k].push_back(generated_crossword[i * matrix_size + k]);
        }
    }
    int matches = 0;
    int counter = 0;
    for(auto& s : grid){
        matches += ahotrie.process(s,counter,matrix_size);
        counter++;

    }
    counter = 0;
    for(auto& s : grid2){
        matches += ahotrie.process(s,counter,matrix_size,false,true);
        counter++;
    }
    for(int i=-matrix_size+1;i<matrix_size;i++){
        string s = "";
        string s2 = "";
        for(int j=0;j<matrix_size;j++){
            if(i + j >= 0 && j+i<matrix_size){
                s+=grid[j+i][j];
                s2+=grid[j+i][matrix_size - 1 - j];
            }
        }
        matches += ahotrie.process(s,i,matrix_size,true,true);
        matches += ahotrie.process(s2,i,matrix_size,true);
    }
}

bool generate_crossword(const int matrix_size, const vector<string> &dictionar, const string &_seed, vector<string> &solutions) {
	auto seed = strtoull(_seed.c_str(), NULL, 16) & 0x7fffffff;
    mt19937 gen(seed);
    srand(seed);
	auto solution = solutions[rand() % solutions.size()];
    Aho ahotrie {};
    vector<char> generated_crossword(matrix_size * matrix_size, '#');
    vector<int> choice_x(matrix_size);
    iota(choice_x.begin(), choice_x.end(), 0);
    istringstream iss(solution);
    vector<string> words((istream_iterator<string>(iss)), istream_iterator<string>());// Split solution into words
    vector<pair<int,string>> words_with_index;
	vector<string>reverse_words;
    for (const string &word : words) {
        if (choice_x.empty()) {
            panic("Nedostatek mista pro slova");
        }
        uniform_int_distribution<> dis(0, choice_x.size() - 1);
        int random_x = choice_x[dis(gen)];
        choice_x.erase(remove(choice_x.begin(), choice_x.end(), random_x), choice_x.end());
        int increment = matrix_size / word.size();
        int y = 0;
        words_with_index.push_back({random_x,word});
        if(increment==0){
            panic("Nedostatek mista pro slovo");
        }
        for (size_t i = 0; i < word.size(); i++) {
            generated_crossword[random_x * matrix_size + y] = word[i];
            ahotrie.placed_chars.push_back({{random_x,y},word[i]});
            // cerr<<random_x<<" "<<y<<" "<<word[i]<<endl;
            y += increment;
        }
    }

    sort(words_with_index.begin(),words_with_index.end());
    string solution_str="";
    for(auto word:words_with_index){
        solution_str+=word.second+" ";
    }
    set<string> chosen_words;
    vector<int> word_heuristic = calc_heuristic(dictionar);
    
    vector<pair<int, int>> directions = {{0, 1}, {1, 0}, {1, 1}, {-1, -1}};
    for(int i = 0;i<matrix_size;i++){
        for(int j=0;j<matrix_size;j++){
            for (auto dir : directions) {
                int freespace = calc_freespace(i, j, dir.first, dir.second, generated_crossword, matrix_size);
                if (freespace < MIN_WORD_SIZE) {
                    continue;
                }
                string word = find_word(dictionar, word_heuristic, freespace);
                if (word == "#") {
                    continue;
                }
                chosen_words.insert(word);
                if(gen()%4==0){
                    reverse(word.begin(), word.end());
                    reverse_words.push_back(word);
                    shuffle(directions.begin(), directions.end(), gen);
                }
                place_word(word, i, j, dir.first, dir.second, generated_crossword, matrix_size);
                ahotrie.placed_words[word].push_back(Position({i,j},dir.first,dir.second));
                ahotrie.add_string(word);
                reverse(word.begin(), word.end());
                ahotrie.add_string(word);
                
                break;
            }
        }
    }
    ahotrie.prepare();
    
    check_ahotrie(ahotrie,generated_crossword,matrix_size);
    for(auto& p : ahotrie.duplicate_words){
        string reversed=p.first;
        reverse(reversed.begin(),reversed.end());
        if(ahotrie.placed_words.find(reversed)!=ahotrie.placed_words.end()){
            for(int i=0;i<ahotrie.placed_words[reversed].size();i++){
                remove_word(reversed,ahotrie.placed_words[reversed][i],generated_crossword,matrix_size,ahotrie);
            }
        }
        for(int i=0;i<ahotrie.placed_words[p.first].size();i++){
            remove_word(p.first,ahotrie.placed_words[p.first][i],generated_crossword,matrix_size,ahotrie);
        }
        if(chosen_words.find(p.first)!=chosen_words.end()){
            chosen_words.erase(p.first);
        }
        if(chosen_words.find(reversed)!=chosen_words.end()){
            chosen_words.erase(reversed);
        }
    }
    if(q_placed){
        chosen_words.insert("q");
    }
    if(w_placed){
        chosen_words.insert("w");
    }
    if(x_placed){
        chosen_words.insert("x");
    }
    

    vector<string> grid;
    grid.resize(matrix_size);
    for (int i = 0; i < matrix_size; i++) {
        for (int k = 0; k < matrix_size; k++) {
            grid[i].push_back(generated_crossword[i * matrix_size + k]);
            if(generated_crossword[i * matrix_size + k] == '#'){
                hash_placed = true;
            }   
        }
    }
    if(hash_placed){
        chosen_words.insert("#");
    }
    cout<<matrix_size<<" "<<chosen_words.size()<<endl;
    for(int i=0;i<matrix_size;i++){
        for(int j=0;j<matrix_size;j++){
            cout<<grid[i][j];
        }
        cout<<endl;
        
    }
    
    vector<string> chosen_words_vector;
    for (auto word : chosen_words) {
        chosen_words_vector.push_back(word);
    }
    shuffle(chosen_words_vector.begin(), chosen_words_vector.end(), gen);
    for(auto word:chosen_words_vector){
        cout<<word<<endl;
    }
    return true;    
}

int main(int argc, char *argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    if (argc < 3) {
        fail("Usage: <case> <seed>");
    }

    return 0;
}
