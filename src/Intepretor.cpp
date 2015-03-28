#include "Intepretor.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
using namespace std;
//将string类型的type转换为ColType
ColType Intepretor::Trasn2type(string type)
{
	if (type == "int")
		return ColType::Int;
	if (type == "char")
		return ColType::Char;
	if (type == "float")
		return ColType::Float;
	/*
			如果不属于这三种类型，需要抛出一个错误得救
	*/
	else throw Error(0, "Interpreter", "Type", "类型错误，不支持用户输入的类型!");
}
//将String 转为Int
int Intepretor::String2Int(string s)
{
	int n = atoi(s.c_str());
	return n;
}
//将String 转为 Char *
const char * Intepretor::String2Char(string  s)
{
	const char *n = s.c_str();
	return n;
}
//运算符号选择判断
enum Operator_type  Intepretor::Op_Judge(string Op)
{
	if (Op == "<")
		return EQUAL;
	if (Op == "<")
		return LESS_THAN;//<
	if (Op == "=")
		return EQUAL;//=
	if (Op == ">")
		return MORE_THAN;//>
	if (Op == ">=")
		return MORE_AND_EQUAL;//>=
	if (Op == "<=")
		return LESS_AND_EQUAL;//<=
	if (Op == "!=")
		return NOT_EQUAL;//!=  另 where子句不能使用！=运算符
	else throw Error(0, "Interpreter", "Operator", "不支持用户输入的运算符!");
}
/*
	命令
*/
//获得输入的命令
void Intepretor::CommandInput()
{
	while (true)
	{
		string command;
		vector<string> Input;
		while (cin >> command)
		{
			//取出（）;,
			bool End;//用户是否输入完成
			auto start = command.begin();
			for (auto i = command.begin(); i != command.end(); i++)
			{
				string s;
				switch (*i)
				{
				case'(':
				case ')':
				case ',':
				case '\'':
				case '=':
					if (start != i)
						Input.push_back(string(start, i));
					s.push_back(*i);
					Input.push_back(s);
					start = i;
					start++;
					break;
				case '<':
				case '>':
					if (start != i)
						Input.push_back(string(start, i));
					s.push_back(*i);
					i++;
					if (i != command.end() && (*i == '=' || *i == '>'))
						s.push_back(*i);
					else
						i--;
					Input.push_back(s);
					start = i;
					start++;
					break;
				case ';':
					if (start != i)
						Input.push_back(string(start, i));
					start = i;
					start++;
					End = true;
					break;

				default:
					break;
				}
				if (End == true)
					break;
			}
			if (Is_Quit(Input))
				exit(0);//退出
			else
				ParseCommand();

		}
	}
}
//解析命令
void Intepretor::ParseCommand()
{
	if (Is_CreateTable(Input))
		CreateTable_command(Input);
	if (Is_Select(Input))
		Select_command(Input);
	if (Is_Insert(Input))
		Insert_command(Input);
}
//是否为创建数据表命令
bool Intepretor::Is_CreateTable(vector<string> input)
{
	if (input.size() >= 2 && input[0] == "create"&&input[1] == "table")
		return true;
	else
		return false;
}
//是否为选择命令
bool Intepretor::Is_Select(vector<string> input)
{
	if (input.size() >= 2 && input[0] == "insert"&&input[1] == "into")
		return true;
	else
		return false;
}
bool Intepretor::Is_Quit(vector<string> input)
{
	if (input.size() >= 2 && input[0] == "quit")
		return true;
	else
		return false;
}
//创建数据表
void Intepretor::CreateTable_command(vector<string>Input)
{
	Command_State state = Create;
	Table_Type table;//新建立的数据表
	vector<Column_Type> NewTableColumn;//新的数据表中字段
	Column_Type column;
	Column_Type columnprimary;//primary key 约束需要的字段名变量
	for (auto i = Input.begin(); i != Input.end(); i++)
	{
		switch (state)
		{
		case Create:
			state = Table; break;
		case Table:
			state = Table_Name; break;
		case Table_Name:
			state = Left_Query;
			table.Table_Name = *i;
			break;
		case Left_Query:
			if (*i != "(")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			state = Column_Name;
			break;
		case Column_Name://字段名
			state = Column_type;
			column.Column_TypeName = *i;
			break;
		case Column_type://字段属性
			column.coltype = Trasn2type(*i);
			if (Trasn2type(*i) == Char)
				state = Char_LeftBrackets;
			else
				state = Unique;
			break;
		case Char_LeftBrackets://左括号
			if (*i != "(")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			state = CharSize;
			break;
		case CharSize://Char类型大小
			column.RequestSize = String2Int(*i);
			state = Char_LeftBrackets;//char的左括号
			break;
		case Char_RightBrackets://右括号
			if (*i != ")")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			if (*(++i) == "not")
				state = Not_Null_not;
			break;
		case Not_Null_not://不为空
			if (*i != "not")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			state = Not_Null_null;
			break;
		case Not_Null_null://不为空
			if (*i != "null")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			column.IsNotNull = true;
			if (*(++i) == "unique")
				state = Unique;
			break;
		case Unique://唯一属性
			if (*i != "unique")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			column.IsUnique = true;
			if (*(++i) == ",")
			{
				state = ColumnEndComma;
				NewTableColumn.push_back(column);
			}	
			break;
		case ColumnEndComma://一个属性结束，至一个新的属性
			state = Column_Name;
			break;
		case PrimaryKey_primary://主键
			if (*i != "primary")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			state = PrimaryKey_key;
			break;
		case PrimaryKey_key://主键
			if (*i != "key")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			state = PrimaryKey_LeftBrackets;
			break;
		case PrimaryKey_LeftBrackets://主键左括号
			if (*i != "(")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			state = PrimaryKey_ColumnName;
			break;
		case PrimaryKey_ColumnName://主键属性名
			columnprimary = Catalog::Get_Column(table.Table_Name,*i);
			columnprimary.IsPrimary = true;
			if (*(++i) == ")")
				state = PrimaryKey_RightBrackets;
			if (*(++i) == ",")
				state = PrimaryKey_Comma;
			break;
		case PrimaryKey_Comma://继续保持主键的属性的状态
			state = PrimaryKey_ColumnName;
			break;
		case PrimaryKey_RightBrackets://主键右括号
			if (*i != ")")
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			if (*(++i) == ")")
				state = EndPrimaryKey;
			break;
		case EndPrimaryKey://主键结束
			if (*i == ")")
				state = Right_Query;
			else
				throw Error(0, "Interpreter", "Create table", "语法错误!");
			break;
		case Right_Query://建表结束，传递用户输入的参数
			Catalog::Instance().CheckTable(table.Table_Name,NewTableColumn);//传到Catalog进行Table Check操作
			API::Instance().CreateTable(table.Table_Name, NewTableColumn);//传到API进行操作
			break;
		default:
			break;
		}
	}
}
/*
	选择命令接口的构造函数
*/
//选择Select命令
void Intepretor::Select_command(vector<string> Input)
{
	Command_State State = Select;
	string Attribute[100] = {"$"}; int j = 0;//选择的属性
	string FromLists[100] = {"$"}; int k = 0;//选择的数据表
	WhereList WhereLists[100]; int w = 0;//where
	for (auto i = Input.begin(); i != Input.end(); i++)
	{
		switch (State)
		{
		case Select:
			State = SelList;
			break;
		case SelList:
			if (*i == "*")
			{
				Attribute[j] = "All";//如果是* 即选择所有的属性
				break;
			}
			Attribute[j] = *i;
			j++;
			if (*(++i) == "from")
				State = From;
			if (*(++i) == ",")
				State = SelList;
			if (*(++i) == "from")
				State = From;
			break;
		case From:
			if (*i != "from")
				throw Error(0, "Interpreter", "Select", "语法错误!");
			State = FromList;
			break;
		case FromList:
			FromLists[k] = *i;
			k++;
			if (*(++i) == ",")
				State = FromList;
			if (*(++i) == "where")
				State = Where;
			break;
		case Where:
			WhereLists[w].Attribute = *i;
			i++;
			WhereLists[w].Where_Operator = *i;//运算符
			i++;
			if (*i == "'")
				WhereLists[w].StrValue = String2Char(*(++i));
			else
				WhereLists[w].IntValue = String2Int(*(++i));
			if (*i == "and")
				State = And;
			if (*i == ";")
				State = EndSelect;
			break;
		case And:
			State = Where;
			break;
		case EndSelect:
			Selection::Selection();

			break;
		default:
			break; 
		}
	}
}

/*
	Insert_into 命令解析
*/
void Intepretor::Insert_command(vector<string> input)
{
	Insert_IntoStruct insertintovalues;
	Command_State state = Insert;//插入状态
	string Inserttable;//插入的数据表
	for (auto i = input.begin(); i != input.end(); i++)
	{
		switch (state)
		{
		case Insert:
			state = Into;
			break;
		case Into:
			state = InsertTable;
			break;
		case InsertTable:
			state = Insert_Value;
			Inserttable = *i;
			break;
		case Insert_Value:
			state = Insert_Leftbracket;
			break;
		case Insert_Leftbracket:
			if (*i != "(")
				throw Error(0, "Interpreter", "Insert into", "语法错误!");
			state = Insert_Values;
			break;
		case Insert_Values:
			if (*i == "'")
			{
				i++;
				insertintovalues.CharValues = *i;
			}
			else
				insertintovalues.IntValues = String2Int(*i);
			if (*(++i) == ",")
				state = Insert_Values;
			if (*(i) == ")")
				state = Insert_Rightbracket;
			else
				throw Error(0, "Interpreter", "Insert into", "语法错误!");
			break;
		case Insert_Rightbracket:
			if (*i == ";")
				state = EndInsert;
			else
				throw Error(0, "Interpreter", "Insert into", "语法错误!");
			break;
		case EndInsert:
			break;
		default:
			break;
		}
	}
}