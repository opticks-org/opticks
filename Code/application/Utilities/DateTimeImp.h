/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef DATETIMEIMP_H
#define DATETIMEIMP_H

#include "DateTime.h"

class DateTimeImp : public DateTime
{
public:
   DateTimeImp();
   DateTimeImp(const DateTimeImp& rhs);
   DateTimeImp(const time_t& fromTime);
   DateTimeImp(const std::string &fromTime);
   virtual ~DateTimeImp();

   virtual DateTimeImp& operator =(const DateTimeImp& rhs);
   virtual bool operator ==(const DateTimeImp& rhs) const;
   virtual bool operator !=(const DateTimeImp& rhs) const;

   // Accessors
   std::string getFormattedLocal(const std::string& fmt) const;
   std::string getFormattedUtc(const std::string& fmt) const;
   time_t getStructured() const;
   double getSecondsSince(const DateTime& other) const;

   // Mutators
   virtual void setStructured(time_t val);
   virtual void setToCurrentTime();
   virtual bool set(unsigned short year, unsigned short month, unsigned short day,
      unsigned short hour, unsigned short minute, unsigned short second);
   virtual bool set(unsigned short year = 2000, unsigned short month = 01, unsigned short day = 01);
   virtual bool set(const std::string &fromTime);
   virtual bool isTimeValid() const;
   virtual bool isValid() const;

   static int getMonth(const std::string& month);
   static int getDay(const std::string& day);

private:
   unsigned int mTime;
   bool mOnlyDateIsValid;
};

#endif
