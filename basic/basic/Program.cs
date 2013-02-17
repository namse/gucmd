using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;


namespace basic
{
    public class ErrorCart
    {
        public string strErrorType;
        public List<string> listErrors;
        public ErrorCart(string type )
        {
            strErrorType = type;
            listErrors = new List<string>();
        }
        public bool IsDupError(string error)
        {
            foreach (string _error in listErrors)
            {
                if (string.Compare(_error, error) == 0)
                    return true;
            }
            return false;
        }
        public bool AddError(string error) 
        {
            if (IsDupError(error) == false) // 똑같은 내용의 에러가 잇는지 확인 
            {
                listErrors.Add(error);
                return true;
            }
            return false;
        }
    }
    public class DataBase
    {
        public List<ErrorCart> listEC;
        public DataBase()
        {
            listEC = new List<ErrorCart>();
        }
        public bool AddError(string type, string error) // false -> 중복된거.
        {
            foreach (ErrorCart _ec in listEC)
            {
                if (string.Compare(type, _ec.strErrorType) == 0) // 이미 똑같은 타입의 에러가 저장되어있는지 확인합니다.
                {
                    return _ec.AddError(error); //에러의 내용을 카트리지에 추가시킵니다.
                    
                }
            }
            ErrorCart ec = new ErrorCart(type); // 똑같은 타입의 에러가 저장되어지지 않았을 때. 새로운 카드리지를 만들고 저장.
            listEC.Add(ec);
            return ec.AddError(error);
        }
    }
    class Program
    {
        static void Main(string[] args)
        {
            DataBase dbError = new DataBase();
            Console.WriteLine(@"Enter errorLog Directory : ");
            string directory = Console.ReadLine(); // error_log파일의 path를 입력합니다.

            StreamReader reader = new StreamReader(directory);
            string strFileLine = string.Empty;
            while ((strFileLine = reader.ReadLine()) != null)
            {
                Regex rx = new Regex(@"(?<=\[.*?\] \[error\] \[.*?\] ).*?:"); // 후방 및 게으른 수량자로 [날짜][에러][클라이언트]와 : 사이의 에러명을 구한다.
                if (rx.IsMatch(strFileLine) == true)
                {
                    string strErrorType = rx.Match(strFileLine).Value; // 에러의 종류
                    int A = rx.Match(strFileLine).Index + rx.Match(strFileLine).Length+1;
                    string strError = strFileLine.Substring(A); // 에러의 내용
                    dbError.AddError(strErrorType, strError); // 데이터베이스에 저장시킵니다.
                }
            }
            Console.WriteLine(@"Detail Error Report");
            int i = 1;
            foreach (ErrorCart ec in dbError.listEC)
            {
                Console.WriteLine(@"-----------------------------------------------------");
                Console.WriteLine(@"{0}.{1}",i++,ec.strErrorType);
                foreach (String error in ec.listErrors)
                {
                    Console.WriteLine(@"{0}",error);
                }
            }
            Console.WriteLine(@"Need .txt log File? Y or N");
            string strNeed = Console.ReadLine();
            if (String.Compare(strNeed, @"Y") == 0)
            {
                StreamWriter writer = File.CreateText(Path.GetDirectoryName(directory)+Path.GetFileNameWithoutExtension(directory)+@".txt");
                i = 1;
                writer.WriteLine(@"Detail Error Report");
                foreach (ErrorCart ec in dbError.listEC)
                {
                    writer.WriteLine(@"-----------------------------------------------------");
                    writer.WriteLine(@"{0}.{1}", i++, ec.strErrorType);
                    foreach (String error in ec.listErrors)
                    {
                        writer.WriteLine(@"{0}", error);
                    }
                }
                writer.Flush();
                writer.Close();
            }
            reader.Close();
        }
    }
}
