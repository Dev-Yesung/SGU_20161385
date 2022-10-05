#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;

void solution1(string fileName, string idx);
void solution2(string fileName, string idx);
void solution3(string fileName, string idx);
int findMaxSum(vector<int>& vec);

int main(int argc, char** argv) {
	// 실행할 알고리즘 index를 변수에 복사
	string cmdIdx = argv[2];
	if(cmdIdx == "1") {
		// 1번 알고리즘: O(N^6)
		solution1(argv[1], cmdIdx);		
	}
	else if(cmdIdx == "2") {
		// 2번 알고리즘: O(N^4)
		solution2(argv[1], cmdIdx);		
	}
	else if(cmdIdx == "3") {
		// 3번 알고리즘: O(N^3)
		solution3(argv[1], cmdIdx);		
	}
	
	return 0;
}

// 1번 알고리즘 : O(N^6)
void solution1(string fileName, string idx) {
	// 파일입력을 위한 파일스트림 생성
	ifstream ifs;
	ifs.open(fileName);

	if(ifs.fail()) {
		cerr << "File open error!\n";
		return;
	}
	
	int  row, col;
	ifs >> row;
	ifs >> col;
	// 파일로부터 받은 값을 동적 컨테이너인 vector에 저장
	// 이때 2차원 vector의 row의 길이는 정해서 초기화해야 한다
	vector<vector<long long>> matrix(row);
	long long val;
	for(int i = 0; i < row; i++)
		for(int j = 0; j < col; j++) {
			ifs >> val;
			matrix[i].push_back(val);
		}
	// 시간측정 시작위치
	clock_t startTime = clock();
	// 최댓값을 구하는 알고리즘
	int total = 0, maxSum = 0;
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
			for(int k = i; k < row; k++) {
				for(int l = j; l < col; l++) {
					for(int m = i; m <= k; m++) {
						for(int n = j; n <= l; n++) {
							total += matrix[m][n];
						}
					}
					if(total > maxSum)
						maxSum = total;
					total = 0;
				}
			}
		}
	}
	// 시간측정 완료위치
	clock_t endTime = clock();
	clock_t elapsed = endTime - startTime;
	// 파일출력을 위한 파일스트림 생성
	string outputName;
	outputName.append("result_").append(fileName);

	ofstream ofs;
	ofs.open(outputName);

	if(ofs.fail()) {
		cerr << "File output error!\n";
		return;
	}

	//파일에 결과값 출력
	ofs << fileName << '\n'
		<< idx << '\n'
		<< row << '\n'
		<< col << '\n'
		<< maxSum << '\n'
		<< elapsed << '\n';

	ifs.close();
	ofs.close();
}

// 2번 알고리즘 : O(N^4)
void solution2(string fileName, string idx) {
	// 파일입력을 위한 파일스트림 생성
	ifstream ifs;
	ifs.open(fileName);

	if(ifs.fail()) {
		cerr << "File open error!\n";
		return;
	}
	
	int row, col;
	ifs >> row;
	ifs >> col;
	
	vector<vector<long long>> matrix(row);
	long long val;
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
			ifs >> val;
			matrix[i].push_back(val);
		}
	}
	// 시간측정 시작위치
	clock_t startTime = clock();
	int maxSum = -999999;
	// 여기서 O(N^2)의 시간복잡도를 갖는다
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
			// 배열에서 memoization을 위한 계산을 수행(row에 대한 계산)
			if(i > 0)
				matrix[i][j] += matrix[i-1][j];
			// 배열에서 memoization을 위한 계산을 수행(col에 대한 계산)
			if(j > 0)
				matrix[i][j] += matrix[i][j-1];
			// 나머지 사각형에 값 계산
			if(i > 0 && j > 0)
				matrix[i][j] -= matrix[i-1][j-1];
		}
	}

	// 최대값을 찾는 알고리즘으로 이 부분에서 O(N^4)의 시간복잡도를 갖는다
	// i의 값은 row로 처음 사각형의 시작위치를 정할 때 사용한다
	for(int i = 0; i < row; i++) {
		// j의 값은 col로 처음 사각형의 시작위치를 정할 때 사용한다 
		for(int j = 0; j < col; j++) {
			// k의 값은 (i, j)사각형에서 시작해서 확장하는 row의 길이이다
			for(int k = i; k < row; k++) {
				// l의 값은 (i, j)사각형에서 시작해서 확장하는 col의 길이이다
				for(int l = j; l < col; l++) {
					// sumOfSubRectangle의 값은 (k, l)의 값으로 확장한 사각형을 나타낸다
					int sumOfSubRectangle = matrix[k][l];

					// (i, j)에서 (k, l)까지 사각형의 크기를 구하기 위한 계산 값들
					if(i > 0)
						// 구하고자 하는 사각형에서 빼야할 가로 사각형의 값
						sumOfSubRectangle -= matrix[i-1][l];
					if(j > 0)
						// 구하고자 하는 사각형에서 빼야할 세로 사각형의 값
						sumOfSubRectangle -= matrix[k][j-1];
					if(i > 0 && j > 0)
						// 뺀 두 개의 가로, 세로 사각형에서 두 번 빼진 값을 보정한다
						sumOfSubRectangle += matrix[i-1][j-1];
					// 사각형 계산 결과의 최대값을 저장해놓는다
					if(sumOfSubRectangle > maxSum)
						maxSum = sumOfSubRectangle;
				}
			}
		}
	}
	// 시간측정 완료위치
	clock_t endTime = clock();
	clock_t elapsed = endTime - startTime;
	// 파일출력을 위한 파일스트림 생성
	string outputName;
	outputName.append("result_").append(fileName);

	ofstream ofs;
	ofs.open(outputName);

	if(ofs.fail()) {
		cerr << "File output error!\n";
		return;
	}

	//파일에 결과값 출력
	ofs << fileName << '\n'
		<< idx << '\n'
		<< row << '\n'
		<< col << '\n'
		<< maxSum << '\n'
		<< elapsed << '\n';

	ifs.close();
	ofs.close();
}

// 3번 알고리즘 : O(N^3)
void solution3(string fileName, string idx) {
	// 파일입력을 위한 파일스트림 생성
	ifstream ifs;
	ifs.open(fileName);

	if(ifs.fail()) {
		cerr << "File open error!\n";
		return;
	}
	
	int row, col;
	ifs >> row;
	ifs >> col;
	
	vector<vector<long long>> matrix(row);
	long long val;
	for(int i = 0; i < row; i++) {
		for(int j = 0; j < col; j++) {
			ifs >> val;
			matrix[i].push_back(val);
		}
	}
	// 시간측정 시작위치
	clock_t startTime = clock();
	// 최댓값을 구하는 알고리즘
	int maxSum = -999999;

	// 1차원 배열에서 사용했던 Kadane알고리즘을 2차원 배열에 응용하였다
	// 여기에서도 memoization을 사용한다. 다만 원래 데이터를 저장해놨던 배열을 수정할 필요가 없다.
	// 첫 번째 left_col에 대한 for문은 시작 column의 위치를 지정한다 
	// left_col의 위치부터 kadane알고리즘을 시작한다
	for(int left_col = 0; left_col < col; left_col++) {
		// 결과값을 vector를 생성한다
		vector<int> between_size(row, 0);
		// right_col은 left_col에서부터 2차원 배열에 저장된 값을 더해가며 확장을 한다
		for(int right_col = left_col; right_col < col; right_col++) {
			for(int r = 0; r < row; r++) {
				// r의 값은 배열의 row index를 의미하며 row별로 값을 더해가며
				// memoization을 사용하기 위해 vector안에 값을 더해간다
				between_size[r] += matrix[r][right_col];
			}
			// findMaxSum은 확장된 column까지의 최대값을 판별할 때 사용하는 함수이다
			// 확장된 범위 중에 값이 최대인 사각형을 찾는다
			// 기존의 최대값이었던 maxSum값과 비교 후 더 크다면 함수의 리턴값을 maxSum으로 한다
			// 만일 리턴값이 maxSum보다 작다면 기존의 사각형이 최대값에 해당하는 것이다
			maxSum = max(maxSum, findMaxSum(between_size));
		}
	}
	// 시간측정 완료위치
	clock_t endTime = clock();
	clock_t elapsed = endTime - startTime;
	// 파일출력을 위한 파일스트림 생성
	string outputName;
	outputName.append("result_").append(fileName);

	ofstream ofs;
	ofs.open(outputName);

	if(ofs.fail()) {
		cerr << "File output error!\n";
		return;
	}

	//파일에 결과값 출력
	ofs << fileName << '\n'
		<< idx << '\n'
		<< row << '\n'
		<< col << '\n'
		<< maxSum << '\n'
		<< elapsed << '\n';

	ifs.close();
	ofs.close();
}

// 확장된 범위에서 제일 큰 사각형을 찾는 함수
int findMaxSum(vector<int>& vec) {
	/*
	   Kadane알고리즘을 이용한다
	   Kadane알고리즘은 기존에 더해져 있던 수에 새로 들어온 수를 간단히 더하면 되기 때문에
	   연속된 범위에서 합의 최대값을 선형시간으로 구할 수 있는 알고리즘이다
	   이 함수가 call되기 전에 반복문은 2번 거치고 함수 내부에서 반복문은 1번 거친다
	   따라서 O(N^3)의 upper bound를 갖는 알고리즘이 된다
	*/
	
	// sum은 연속된 사각형의 합을 구할 때 사용한다
	int sum = vec[0];
	// maxSum은 연속된 사각형 중에 가장 큰 값을 저장하고 sum과 비교해서
	// 확장된 범위 내에서 가장 큰 수로 정해진다
	int maxSum = sum;
	// 연속된 사각형 값이 저장된 vector에서 index 0번을 제외한 비교를 시작한다
	// index 0은 sum에 이미 값이 들어있기 때문이다
	for(int i = 1; i < vec.size(); i++) {
		/*	
			sum값은 시작위치를 나타내는데, 만일 sum+vec[i] 보다 vec[i]의 값이 더 크다면 최대값 측정 최대위치를 바꾼다. 
			즉, 원래 sum이 0번 row부터 아래로 내려가며 연속된 값을 가지고 최대값을 찾았다면 vec[2]의 값이 더 큰 경우 
			2번 row의 위치부터 아래로 내려가며 연속된 사각형의 최대값을 찾아나간다
		*/
		if(sum + vec[i] > vec[i]) {
			sum = sum + vec[i];
		} else {
			sum = vec[i];
		}
		maxSum = max(sum, maxSum);
	}
	// 범위 내의 가장 큰 사각형의 값을 리턴
	return maxSum;
}
