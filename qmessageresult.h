#ifndef QMESSAGERESULT_H
#define QMESSAGERESULT_H

#include "QtEventBus_global.h"

#include <QSharedData>
#include <QtPromise>

template<typename T>
class QMessageMeta
{
public:
    static constexpr bool external = false;
    static constexpr bool stick = false;
    static constexpr char const * topic = nullptr;
};

// register properties of this message type

#define Q_MESSAGE_META(ty, et, sk, tp) Q_MESSAGE_META_RESULT(ty, et, sk, tp, void)

#define Q_MESSAGE_META_RESULT(ty, et, sk, tp, rt) \
    Q_DECLARE_METATYPE(ty) \
    template<> \
    class QMessageMeta<ty> \
    { \
    public: \
        static constexpr bool external = et; \
        static constexpr bool stick = sk; \
        static constexpr char const * topic = tp; \
        typedef rt result; \
    };

template<typename R>
struct VectorResult
{
    typedef QVector<R> type;

    static QVariant toVar(R const & result) {
        return QVariant::fromValue(result);
    }

    template<typename F>
    static R fromVar(F const & f, QByteArray const & t, QVariant const & v) {
        QVariant r = f(t, v);
        return r.value<R>();
    }

    static QtPromise::QPromise<QVector<QVariant>> mapResult(QtPromise::QPromise<QVector<R>> p) {
        return p.map([](R const & r, int) { return toVar(r); });
    }

    static QtPromise::QPromise<QVector<R>> empty() {
        return QtPromise::QPromise<QVector<R>>::resolve({});
    }
};

template<>
struct VectorResult<void>
{
    typedef void type;

    template<typename F>
    static void fromVar(F const & f, QByteArray const & t, QVariant const & v) {
        f(t, v);
    }

    static QtPromise::QPromise<QVector<QVariant>> mapResult(QtPromise::QPromise<void> p) {
        return p.then([] () { return QVector<QVariant>{}; });
    }

    static QtPromise::QPromise<void> empty() {
        return QtPromise::QPromise<void>::resolve();
    }
};

template<typename R>
struct PromiseResolver {
    QtPromise::QPromiseResolve<R> const resolve;
    QtPromise::QPromiseReject<R> const reject;
};

template<typename R, typename T, typename F, typename RR = typename std::result_of<F(T &)>::type>
struct QMessageResultResolve {
    static void invoke(PromiseResolver<R> const & rl, F const & f, T const & msg) {
        rl.resolve(f(msg));
    }
};

template<typename R, typename T, typename F>
struct QMessageResultResolve<R, T, F, QtPromise::QPromise<R>> {
    static void invoke(PromiseResolver<R> const & rl, F const & f, T const & msg) {
        f(msg).then([r = rl.resolve](R const & d) {
            r(d);
        }, rl.reject);
    }
};

template<typename T, typename F, typename RR>
struct QMessageResultResolve<void, T, F, RR> {
    static void invoke(PromiseResolver<void> const & rl, F const & f, T const & msg) {
        f(msg);
        rl.resolve();
    }
};

template<typename T, typename F>
struct QMessageResultResolve<void, T, F, QtPromise::QPromise<void>> {
    static void invoke(PromiseResolver<void> const & rl, F const & f, T const & msg) {
        f(msg).then(rl.resolve, rl.reject);
    }
};

template<typename T, typename R>
struct QMessageResult
{
    T const msg;
    mutable std::atomic<int> index{0};
    QVector<PromiseResolver<R>> resolvers;

    QMessageResult(T const msg) : msg(msg) {}

    QtPromise::QPromise<typename VectorResult<R>::type> await(int n) {
        QVector<QtPromise::QPromise<R>> promises;
        for (int i = 0; i < n; ++i) {
            promises.append(QtPromise::QPromise<R>([this] (auto const & resolve, auto const & reject) {
                resolvers.append({resolve, reject});
            }));
        }
        return QtPromise::all(promises);
    }

    template<typename F>
    void invoke(F const & f) const {
        PromiseResolver<R> const & rl = resolvers[index++];
        try {
            QMessageResultResolve<R, T, F>::invoke(rl, f, msg);
        }  catch (...) {
            rl.reject(std::current_exception());
        }
    }

    template<typename F>
    void invoke2(F const & f) const {
        try {
            f(msg);
        }  catch (...) {
        }
    }
};

template<typename T, typename R>
class QMessageResultSharedData : public QSharedData
{
public:
    QMessageResultSharedData(T const & msg)
        : result_(msg)
    {
    }
    QMessageResult<T, R> result_;
};

class QMessageResultPointer
{
public:
    QMessageResultPointer() {}

    template<typename T>
    QMessageResultPointer(T const & msg)
        : pointer_(new QMessageResultSharedData<T, typename QMessageMeta<T>::result>(msg))
    {
    }

    QMessageResultPointer(QVariant const & msg)
        : pointer_(new QMessageResultSharedData<QVariant, QVariant>(msg))
    {
    }

public:
    template<typename T, typename R>
    QtPromise::QPromise<typename VectorResult<R>::type> await(int n) {
        return static_cast<QMessageResultSharedData<T, R> &>(*pointer_).result_.await(n);
    }

    template<typename T, typename R, typename F>
    void invoke(F const & f) const
    {
        return static_cast<QMessageResultSharedData<T, R> const &>(*pointer_).result_.invoke(f);
    }

    template<typename T, typename R, typename F>
    void invoke2(F const & f) const
    {
        return static_cast<QMessageResultSharedData<T, R> const &>(*pointer_).result_.invoke2(f);
    }

private:
    QSharedDataPointer<QSharedData> pointer_;
};

#endif // QMESSAGERESULT_H
