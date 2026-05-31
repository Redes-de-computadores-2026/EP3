#include "instrumentacao.hpp"

#include <stdexcept>
#include <utility>

Instrumentacao::Instrumentacao() {
    estado_ = std::make_shared<EstadoCompartilhado>();
    prefixo_ = "";
}

Instrumentacao::Instrumentacao(std::shared_ptr<EstadoCompartilhado> estado, std::string prefixo) {
    estado_ = std::move(estado);
    prefixo_ = std::move(prefixo);
}

Instrumentacao Instrumentacao::criar_subinstrumentacao(std::string_view nome) const {
    validar_nome(nome);

    if (prefixo_.empty()) {
        return Instrumentacao(estado_, std::string(nome));
    }

    return Instrumentacao(estado_, prefixo_ + "." + std::string(nome));
}

void Instrumentacao::incrementar(std::string_view nome, valor_t delta) {
    validar_nome(nome);

    const auto chave = nome_completo(nome);

    std::lock_guard<std::mutex> bloqueio(estado_->mutex);
    estado_->metricas[chave] += delta;
}

void Instrumentacao::definir(std::string_view nome, valor_t valor) {
    validar_nome(nome);

    const auto chave = nome_completo(nome);

    std::lock_guard<std::mutex> bloqueio(estado_->mutex);
    estado_->metricas[chave] = valor;
}

std::optional<Instrumentacao::valor_t> Instrumentacao::obter(std::string_view nome) const {
    validar_nome(nome);

    const auto chave = nome_completo(nome);

    std::lock_guard<std::mutex> bloqueio(estado_->mutex);
    auto it = estado_->metricas.find(chave);
    if (it == estado_->metricas.end()) {
        return std::nullopt;
    }

    return it->second;
}

Instrumentacao::valor_t Instrumentacao::obter_ou_zero(std::string_view nome) const {
    auto valor = obter(nome);
    if (!valor.has_value()) {
        return 0;
    }

    return *valor;
}

bool Instrumentacao::existe(std::string_view nome) const {
    return obter(nome).has_value();
}

std::map<std::string, Instrumentacao::valor_t> Instrumentacao::exportar() const {
    std::lock_guard<std::mutex> bloqueio(estado_->mutex);
    return estado_->metricas;
}

void Instrumentacao::limpar() {
    std::lock_guard<std::mutex> bloqueio(estado_->mutex);
    estado_->metricas.clear();
}

std::string Instrumentacao::nome_completo(std::string_view nome) const {
    if (prefixo_.empty()) {
        return std::string(nome);
    }

    return prefixo_ + "." + std::string(nome);
}

void Instrumentacao::validar_nome(std::string_view nome) {
    if (nome.empty()) {
        throw std::invalid_argument("nome de metrica vazio");
    }

    if (nome.find('.') != std::string_view::npos) {
        throw std::invalid_argument("nome de metrica nao pode conter '.'");
    }
}
