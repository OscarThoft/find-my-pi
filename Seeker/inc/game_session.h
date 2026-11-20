
class game_session {
public:
    enum Status {
        INACTIVE,
        RUNNING,
        FOUND,
        LOST
    };

    game_session();

    void setStatus(Status s); //INACTIVE, RUNNING, FOUND eller LOST
    Status getStatus() const;

    double getTimeRunning() const; //Returnerer tiden for den nuværende session
    //Hvis spillet er igang returnere den tiden indtil videre, hvis status er blevet sat til FOUND 

private:
    Status status;
    double startTime = 0.0;
    double endTime = 0.0;

};